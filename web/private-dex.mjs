// DragonTap PRIVATE loader metadata reader. Strict, read-only Dalvik DEX inspection.
// No DEX execution, no vendor/profile hash allowlist; accepted structure is validated
// against every PAC before this descriptor may be shipped to a Vita profile.
const LABELS=['common','select0','back','back','bobj','bobj','char','chardemo','charf','effect','demo_00','demo_08','font00','card_preview','gamedata','text00','card'];
const FIXED=[0,1,9,10,11,12,13,14,15];
const NUMBERED=new Map([[2,2],[4,2],[6,2],[7,2],[8,4],[16,3]]);
const TYPES=['act','bin','cnv','dac','rgba','spr','wav'];
const VERSION='dragontap-private-v1';
function readDex(input){
  const b=input instanceof Uint8Array?input:new Uint8Array(input);
  const v=new DataView(b.buffer,b.byteOffset,b.byteLength);
  function need(p,n){if(!Number.isSafeInteger(p)||p<0||!Number.isSafeInteger(n)||n<0||p+n>b.length)throw Error('Truncated/invalid DEX');}
  function u16(p){need(p,2);return v.getUint16(p,true);}
  function u32(p){need(p,4);return v.getUint32(p,true);}
  function leb(p){let n=0;for(let i=0;i<5;i++){need(p,1);const x=b[p++];n+=(x&127)*2**(i*7);if(x<128)return [n,p];}throw Error('Bad DEX ULEB');}
  need(0,112);
  if(String.fromCharCode(...b.slice(0,4))!=='dex\n')throw Error('Not a Dalvik classes.dex');
  const nstr=u32(0x38), ostr=u32(0x3c), nt=u32(0x40), ot=u32(0x44), nm=u32(0x58), om=u32(0x5c), nc=u32(0x60), oc=u32(0x64);
  if(nstr>100000||nt>100000||nm>100000||nc>100000)throw Error('DEX id budget');
  for(const [n,p,w] of [[nstr,ostr,4],[nt,ot,4],[nm,om,8],[nc,oc,32]])need(p,n*w);
  const decoder=new TextDecoder('utf-8',{fatal:false}),strings=[];
  for(let i=0;i<nstr;i++){let p=leb(u32(ostr+i*4))[1],end=p;while(end<b.length&&end-p<=65536&&b[end])end++;need(end,1);if(end-p>65536)throw Error('Overlong DEX string');strings.push(decoder.decode(b.subarray(p,end)));}
  const types=[];for(let i=0;i<nt;i++){const id=u32(ot+i*4);if(id>=nstr)throw Error('Bad DEX type');types.push(strings[id]);}
  const methods=[];for(let i=0;i<nm;i++){const type=u16(om+i*8),name=u32(om+i*8+4);if(type>=nt||name>=nstr)throw Error('Bad DEX method');methods.push([types[type],strings[name]]);}
  const candidates=[];
  for(let c=0;c<nc;c++){
    let p=u32(oc+c*32+24);if(!p)continue;
    const counts=[];for(let k=0;k<4;k++){const [n,q]=leb(p);if(n>100000)throw Error('DEX member budget');counts.push(n);p=q;}
    for(const n of counts.slice(0,2))for(let k=0;k<n;k++){p=leb(p)[1];p=leb(p)[1];}
    for(const n of counts.slice(2)){let mid=0;for(let k=0;k<n;k++){let a=leb(p);mid+=a[0];p=leb(a[1])[1];a=leb(p);const offset=a[0];p=a[1];if(mid>=nm||!offset||methods[mid][1]!=='<clinit>')continue;
      const count=u32(offset+12);if(count>65536)continue;need(offset+16,count*2);
      const words=new Uint16Array(count);for(let j=0;j<count;j++)words[j]=u16(offset+16+j*2);
      const result=initializer(words,strings);if(result)candidates.push(result);
    }}
  }
  if(candidates.length!==1)throw Error('Expected exactly one DragonTap PRIVATE DEX initializer; found '+candidates.length);
  return candidates[0];
}
function initializer(w,strings){
  const aliases=[],assignments=[];
  for(let i=0;i<w.length;i++){const op=w[i]&255;
    if(op===0x1a&&i+2<w.length){const s=strings[w[i+1]];if(s&&/^[0-9A-F]{4}$/.test(s)&&(w[i+2]&255)===0x4d){aliases.push(s);i+=2;continue;}}
    if([0x13,0x14,0x15].includes(op)){const width=op===0x14?3:2;
      if(i+width+1<w.length&&(w[i+width]&255)===0x67){
        const num=op===0x14?(w[i+1]|(w[i+2]<<16))>>>0:op===0x15?(w[i+1]<<16)>>>0:w[i+1];
        assignments.push([w[i+width+1],num]);i+=width+1;continue;
      }
    }
  }
  if(aliases.length!==17||assignments.length<17||aliases[2]!==aliases[3]||aliases[4]!==aliases[5])return null;
  const blocks=[];for(let i=0;i<=assignments.length-17;i++){const part=assignments.slice(i,i+17);if(part.every((x,j)=>j===0||part[j-1][0]+1===x[0]))blocks.push(part.map(x=>x[1]));}
  if(blocks.length!==1)return null;
  const v=blocks[0],a={};
  for(const j of FIXED)a[LABELS[j]]=aliases[j];
  for(const j of NUMBERED.keys())a[LABELS[j]]=aliases[j];
  const type_keys={};for(let i=0;i<7;i++)type_keys[TYPES[i]]=(v[i]^v[10])>>>0;
  const out={format:1,generator:VERSION,aliases:a,
    count_xor:v[11],offset_xor:v[8],size_xor:v[9],
    image_width_xor:v[12],image_height_xor:v[13],
    table_count_xor:v[14],table_position_xor:v[7],
    table_width_xor:v[15],table_height_xor:v[16],wav_size_xor:v[11],type_keys};
  const allNames=[...FIXED,...NUMBERED.keys()].map(j=>aliases[j]);
  if(new Set(allNames).size!==allNames.length||out.count_xor>65535||out.image_width_xor>65535||out.image_height_xor>65535||
     out.table_count_xor>65535||out.table_width_xor>65535||out.table_height_xor>65535||new Set(Object.values(type_keys)).size!==7)return null;
  return out;
}
export function discoverPrivateCodec(dexBytes){return readDex(dexBytes);}
export function canonicalPrivateName(name,codec){
  if(!name.endsWith('.pac')||name.includes('/'))return name;
  const stem=name.slice(0,-4);
  for(const i of FIXED)if(stem===codec.aliases[LABELS[i]])return LABELS[i]+'.pac';
  for(const [i,digits] of NUMBERED){const label=LABELS[i],prefix=codec.aliases[label];
    if(stem.startsWith(prefix)){const suffix=stem.slice(prefix.length);if(suffix.length===digits&&/^[0-9]+$/.test(suffix))return label+suffix+'.pac';}
  }
  return name;
}
export function codecSidecar(codec){
  // Only numeric decode keys cross into the Vita runtime. No executable code,
  // aliases or filesystem paths are included.
  const p={format:1,generator:VERSION};
  for(const key of ['count_xor','offset_xor','size_xor','image_width_xor','image_height_xor','table_count_xor','table_position_xor','table_width_xor','table_height_xor','wav_size_xor'])
    p[key]=codec[key];
  for(const type of TYPES)p['type_'+type]=codec.type_keys[type];
  return p;
}
export async function validatePrivatePac(blob,codec,name){
  if(blob.size<18)throw Error('Truncated protected PAC '+name);
  const head=new DataView(await blob.slice(0,2).arrayBuffer());
  const count=head.getUint16(0,true)^codec.count_xor,base=2+count*16;
  if(!count||count>8192||base>blob.size)throw Error('Protected PAC table outside bounds: '+name);
  const dv=new DataView(await blob.slice(2,base).arrayBuffer());
  const types=new Set(Object.values(codec.type_keys));let recognized=false;
  for(let i=0;i<count;i++){const p=i*16,off=(dv.getUint32(p,true)^codec.offset_xor^i)>>>0,size=(dv.getUint32(p+4,true)^codec.size_xor^i)>>>0;
    if(off>blob.size-base||size>blob.size-base-off)throw Error('Protected PAC record outside bounds: '+name);
    const type=(dv.getUint32(p+8,false)^i)>>>0;
    const literal=dv.getUint8(p+8)===112&&dv.getUint8(p+9)===108&&dv.getUint8(p+10)===116&&dv.getUint8(p+11)===0;
    if(!types.has(type)&&!literal)throw Error('Unknown protected PAC type: '+name);
    recognized=true;
  }
  if(!recognized)throw Error('PAC has no recognized types: '+name);
}
