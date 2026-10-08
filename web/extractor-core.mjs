import {discoverPrivateCodec,canonicalPrivateName,codecSidecar,validatePrivatePac} from './private-dex.mjs';
const MAX_APK_SIZE = 1024 * 1024 * 1024;
const MAX_ENTRY_SIZE = 64 * 1024 * 1024;
const MAX_DATA_SIZE = 512 * 1024 * 1024;
const MAX_CENTRAL_SIZE = 16 * 1024 * 1024;
const MAX_ENTRIES = 8192;
const ZIP32_MAX = 0xffffffff;

const textEncoder = new TextEncoder();
const utf8Decoder = new TextDecoder('utf-8', { fatal: true });

const COMMUNITY_PROFILES = [
  {
    name: 'community14-a210795b', count: 42802, offset: 996678763, size: 47633006,
    types: [2566558864,1246424290,1133921820,1102453101,1111565669,1184565109,2268849933],
    fixed: { '2752':'common','1BC2':'select0','9B28':'effect','59F2':'demo_00','3C90':'demo_08','D0BD':'font00','5D73':'card_preview','D67E':'gamedata','82B7':'text00' },
    numbered: { '0B49':['back',2], 'BDC7':['bobj',2], 'E03B':['char',2], '8AC1':['chardemo',2], 'FAFD':['charf',4], '47DD':['card',3] }
  },
  {
    name: 'community14-es-d594affc', count: 59050, offset: 830950436, size: 2030988207,
    types: [16535934,3988003272,3567234535,1591802006,63084285,1538081446],
    fixed: { '4D7F':'common','B4EB':'select0','B248':'effect','AC3B':'demo_00','8827':'demo_08','4919':'card_preview','EC5A':'gamedata','A602':'text00' },
    numbered: { '0294':['back',2], 'D794':['bobj',2], 'F298':['char',2], 'AE52':['chardemo',2], 'EB21':['charf',4], '6FA6':['card',3] }
  },
  {
    name: 'community14-invasion-05aa0c5e', count: 33839, offset: 1901542107, size: 866934865,
    types: [2931803040,2273195935,2166741075,3888254667,1077762279,356305721],
    fixed: { '9036':'common','7E8F':'select0','1E1C':'effect','97E6':'demo_00','6E24':'demo_08','0708':'card_preview','90EA':'gamedata','D37C':'text00' },
    numbered: { 'F813':['back',2], '17A5':['bobj',2], '0953':['char',2], '364E':['chardemo',2], '91F9':['charf',4], '1A4B':['card',3] }
  },
  {
    name: 'community14-dbfz-11d60c43', count: 0x39ae, offset: 0xafc6643c, size: 0x64ce617b,
    types: [0xa4c74fe3,0xe995397,0x82f9572b,0x10445923,0x4bee884,0x3e602fa3],
    fixed: { '46C3':'common', 'CC4B':'select0', '7D98':'effect', '2B98':'demo_00', 'D6E1':'demo_08', 'B727':'card_preview', 'AC9E':'gamedata', 'D791':'text00' },
    numbered: { '8ED7':['back',2], 'DC70':['bobj',2], '128B':['char',2], 'CD4A':['chardemo',2], '4BD8':['charf',4], 'FDD0':['card',3] }
  }
];

const KNOWN_EXTENSIONS = new Set(['.pac','.ogg','.png','.bmp','.bin','.dat','.db','.dac','.gdt','.cnv','.spr','.act','.plt','.xml']);

function u16(view, off) { return view.getUint16(off, true); }
function u32(view, off) { return view.getUint32(off, true); }
function put16(view, off, value) { view.setUint16(off, value, true); }
function put32(view, off, value) { view.setUint32(off, value >>> 0, true); }
function hex32(n) { return (n >>> 0).toString(16).padStart(8, '0'); }

export function sanitizeProfileName(stem) {
  let name = stem.replace(/[^a-zA-Z0-9_-]/g, '_').replace(/^_+|_+$/g, '');
  if (name.length > 36) name = name.slice(0, 36);
  if (!name) name = 'Mod';
  if (/^(CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])$/i.test(name)) name = `Mod_${name}`;
  return name;
}

export function canonicalName(name, codec = '') {
  if (name.includes('/') || !name.endsWith('.pac')) return name;
  const profiles = codec ? COMMUNITY_PROFILES.filter(p => p.name === codec) : COMMUNITY_PROFILES;
  const stem = name.slice(0, -4);
  const results = [];
  for (const profile of profiles) {
    const fixedKey = Object.keys(profile.fixed).find(k => k.toLowerCase() === stem.toLowerCase());
    if (fixedKey) { results.push(`${profile.fixed[fixedKey]}.pac`); continue; }
    for (const [prefix, spec] of Object.entries(profile.numbered)) {
      const re = new RegExp(`^${prefix}([0-9]{${spec[1]}})$`);
      const m = stem.match(re);
      if (m) { results.push(`${spec[0]}${m[1]}.pac`); break; }
    }
  }
  const unique = [...new Set(results)];
  return unique.length === 1 ? unique[0] : name;
}

export function detectCommunityProfile(names) {
  let best = null, bestScore = 0, tie = false;
  for (const profile of COMMUNITY_PROFILES) {
    let score = 0;
    for (const name of names) if (canonicalName(name, profile.name) !== name) score++;
    if (score > bestScore) { best = profile; bestScore = score; tie = false; }
    else if (score > 0 && score === bestScore) tie = true;
  }
  return tie || !best ? null : best.name;
}

export function characterInventory(namesInput) {
  const names = namesInput instanceof Set ? namesInput : new Set(namesInput);
  let complete = 0, gapSeen = false;
  const incomplete = [], laterAfterGap = [];
  for (let i = 0; i < 100; i++) {
    const idx2 = String(i).padStart(2, '0');
    const idx4 = String(i).padStart(4, '0');
    const triplet = [`char${idx2}.pac`,`chardemo${idx2}.pac`,`charf${idx4}.pac`];
    const present = triplet.filter(x => names.has(x)).length;
    if (present === 0) { gapSeen = true; continue; }
    if (present !== 3) { incomplete.push(i); continue; }
    if (gapSeen) { laterAfterGap.push(i); continue; }
    complete++;
  }
  const unsupported = [...names].filter(n => /^char[0-9]{3,}\.pac$/.test(n) || /^chardemo[0-9]{3,}\.pac$/.test(n) || /^charf[0-9]{5,}\.pac$/.test(n)).sort();
  return {
    count: complete,
    completeIndices: complete ? `00..${String(complete - 1).padStart(2, '0')}` : '',
    incomplete,
    laterAfterGap,
    unsupported,
    runtimeCompatible: complete >= 13 && !incomplete.length && !laterAfterGap.length && !unsupported.length
  };
}

export function assertSafeName(name) {
  if (!name || /[\x00-\x1f\x7f\\:*?"<>|]/.test(name) || textEncoder.encode(name).length > 200) throw new Error(`Unsafe or overly long path: ${name}`);
  for (const part of name.split('/')) {
    if (!part || part === '.' || part === '..' || /[. ]$/.test(part) || /^(CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])([.]|$)/i.test(part) || /^(dbtb_manifest\.json|mod\.json)([.]|$)/i.test(part)) {
      throw new Error(`Unsafe or reserved name: ${name}`);
    }
  }
}

function decodeName(bytes, flags) {
  if (!bytes.length) return '';
  try { return utf8Decoder.decode(bytes); }
  catch {
    if (flags & 0x0800) throw new Error('Invalid UTF-8 filename in APK.');
    let s = '';
    for (const b of bytes) s += b < 128 ? String.fromCharCode(b) : `_${b.toString(16).padStart(2,'0')}`;
    return s;
  }
}

export async function parseZip(file) {
  if (!file || file.size < 22 || file.size > MAX_APK_SIZE) throw new Error('Truncated APK or APK larger than 1 GiB.');
  const tailStart = Math.max(0, file.size - 65557);
  const tail = new Uint8Array(await file.slice(tailStart).arrayBuffer());
  const tv = new DataView(tail.buffer, tail.byteOffset, tail.byteLength);
  let eocd = -1;
  for (let i = tail.length - 22; i >= 0; i--) {
    if (u32(tv, i) === 0x06054b50 && i + 22 + u16(tv, i + 20) === tail.length) { eocd = i; break; }
  }
  if (eocd < 0) throw new Error('Not a valid APK/ZIP archive.');
  const disk = u16(tv,eocd+4), cdDisk = u16(tv,eocd+6), diskEntries=u16(tv,eocd+8), count=u16(tv,eocd+10);
  const cdSize=u32(tv,eocd+12), cdOffset=u32(tv,eocd+16);
  const eocdAbs = tailStart + eocd;
  if (disk || cdDisk || diskEntries !== count || count === 0xffff || count > MAX_ENTRIES || cdSize > MAX_CENTRAL_SIZE || cdOffset + cdSize > eocdAbs) {
    throw new Error('Split ZIP, ZIP64, oversized, or out-of-bounds directory is not supported.');
  }
  const central = new Uint8Array(await file.slice(cdOffset, cdOffset + cdSize).arrayBuffer());
  const cv = new DataView(central.buffer, central.byteOffset, central.byteLength);
  let p = 0;
  const entries = [];
  for (let i = 0; i < count; i++) {
    if (p + 46 > central.length || u32(cv,p) !== 0x02014b50) throw new Error('Damaged ZIP central directory.');
    const versionMade=u16(cv,p+4), flags=u16(cv,p+8), method=u16(cv,p+10), crc=u32(cv,p+16), compressedSize=u32(cv,p+20), size=u32(cv,p+24);
    const nameLen=u16(cv,p+28), extraLen=u16(cv,p+30), commentLen=u16(cv,p+32), diskStart=u16(cv,p+34), externalAttrs=u32(cv,p+38), localOffset=u32(cv,p+42);
    if (diskStart || size === ZIP32_MAX || compressedSize === ZIP32_MAX || localOffset === ZIP32_MAX || localOffset >= cdOffset) throw new Error('Unsupported or out-of-bounds ZIP entry.');
    const end = p + 46 + nameLen + extraLen + commentLen;
    if (end > central.length) throw new Error('Truncated ZIP directory.');
    const nameBytes = central.slice(p + 46, p + 46 + nameLen);
    const name = decodeName(nameBytes, flags);
    entries.push({ index:i, name, flags, method, crc, compressedSize, size, externalAttrs, localOffset, versionMade, directory:name.endsWith('/') });
    p = end;
  }
  if (p !== central.length) throw new Error('Incorrect ZIP directory size.');
  return { file, entries, cdOffset };
}

async function entryDataBlob(zip, entry) {
  if ((entry.flags & 1) !== 0 || ![0,8].includes(entry.method)) throw new Error(`Encrypted entry or unsupported compression: ${entry.name}`);
  const header = new Uint8Array(await zip.file.slice(entry.localOffset, entry.localOffset + 30).arrayBuffer());
  if (header.length !== 30) throw new Error(`Truncated local header: ${entry.name}`);
  const hv = new DataView(header.buffer, header.byteOffset, header.byteLength);
  if (u32(hv,0) !== 0x04034b50) throw new Error(`Invalid local ZIP header: ${entry.name}`);
  const nameLen=u16(hv,26), extraLen=u16(hv,28);
  const dataOffset=entry.localOffset + 30 + nameLen + extraLen;
  if (dataOffset + entry.compressedSize > zip.cdOffset) throw new Error(`Out-of-bounds compressed data: ${entry.name}`);
  const compressed = zip.file.slice(dataOffset, dataOffset + entry.compressedSize);
  let output;
  if (entry.method === 0) output = compressed;
  else {
    let ds;
    try { ds = new DecompressionStream('deflate-raw'); }
    catch { throw new Error('This browser does not support ZIP deflate extraction. Use a current Chrome, Edge, Firefox, or Safari.'); }
    output = await new Response(compressed.stream().pipeThrough(ds)).blob();
  }
  if (output.size !== entry.size) throw new Error(`Damaged APK: decompressed size mismatch for ${entry.name}`);
  const crc = await crc32Blob(output);
  if (crc !== entry.crc) throw new Error(`Damaged APK: CRC check failed for ${entry.name}`);
  return { blob: output, crc };
}

const CRC_TABLE = (() => {
  const t = new Uint32Array(256);
  for (let i=0;i<256;i++) { let c=i; for(let k=0;k<8;k++) c=(c&1)?0xedb88320^(c>>>1):(c>>>1); t[i]=c>>>0; }
  return t;
})();

export function crc32Bytes(bytes, seed=0xffffffff) {
  let crc = seed >>> 0;
  for (const b of bytes) crc = CRC_TABLE[(crc ^ b) & 255] ^ (crc >>> 8);
  return crc >>> 0;
}

export async function crc32Blob(blob) {
  let crc = 0xffffffff;
  const reader = blob.stream().getReader();
  try {
    while (true) {
      const {value,done}=await reader.read(); if(done) break;
      crc = crc32Bytes(value, crc);
    }
  } finally { reader.releaseLock(); }
  return (crc ^ 0xffffffff) >>> 0;
}

class Sha256 {
  constructor(){
    this.h=new Uint32Array([0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19]);
    this.buf=new Uint8Array(64); this.bufLen=0; this.bytes=0;
  }
  update(data){
    this.bytes += data.length;
    let p=0;
    if(this.bufLen){ const n=Math.min(64-this.bufLen,data.length); this.buf.set(data.subarray(0,n),this.bufLen); this.bufLen+=n; p+=n; if(this.bufLen===64){this._block(this.buf);this.bufLen=0;} }
    while(p+64<=data.length){ this._block(data.subarray(p,p+64)); p+=64; }
    if(p<data.length){ this.buf.set(data.subarray(p),0); this.bufLen=data.length-p; }
    return this;
  }
  _block(b){
    const K=Sha256.K, w=new Uint32Array(64), v=new DataView(b.buffer,b.byteOffset,64);
    for(let i=0;i<16;i++)w[i]=v.getUint32(i*4,false);
    for(let i=16;i<64;i++){const x=w[i-15],y=w[i-2],s0=((x>>>7)|(x<<25))^((x>>>18)|(x<<14))^(x>>>3),s1=((y>>>17)|(y<<15))^((y>>>19)|(y<<13))^(y>>>10);w[i]=(w[i-16]+s0+w[i-7]+s1)>>>0;}
    let [a,b0,c,d,e,f,g,h]=this.h;
    for(let i=0;i<64;i++){const S1=((e>>>6)|(e<<26))^((e>>>11)|(e<<21))^((e>>>25)|(e<<7)),ch=(e&f)^((~e)&g),t1=(h+S1+ch+K[i]+w[i])>>>0,S0=((a>>>2)|(a<<30))^((a>>>13)|(a<<19))^((a>>>22)|(a<<10)),maj=(a&b0)^(a&c)^(b0&c),t2=(S0+maj)>>>0;h=g;g=f;f=e;e=(d+t1)>>>0;d=c;c=b0;b0=a;a=(t1+t2)>>>0;}
    this.h[0]=(this.h[0]+a)>>>0;this.h[1]=(this.h[1]+b0)>>>0;this.h[2]=(this.h[2]+c)>>>0;this.h[3]=(this.h[3]+d)>>>0;this.h[4]=(this.h[4]+e)>>>0;this.h[5]=(this.h[5]+f)>>>0;this.h[6]=(this.h[6]+g)>>>0;this.h[7]=(this.h[7]+h)>>>0;
  }
  digestHex(){
    const bitHi=Math.floor(this.bytes/0x20000000)>>>0, bitLo=(this.bytes<<3)>>>0;
    const padLen=this.bufLen<56?56-this.bufLen:120-this.bufLen;
    const pad=new Uint8Array(padLen+8); pad[0]=0x80; const dv=new DataView(pad.buffer); dv.setUint32(padLen,bitHi,false); dv.setUint32(padLen+4,bitLo,false); this.update(pad);
    return [...this.h].map(x=>x.toString(16).padStart(8,'0')).join('');
  }
}
Sha256.K=new Uint32Array([0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2]);

export async function sha256Blob(blob) {
  const sha=new Sha256(), reader=blob.stream().getReader();
  try { while(true){const {value,done}=await reader.read();if(done)break;sha.update(value);} }
  finally { reader.releaseLock(); }
  return sha.digestHex();
}

function extension(name){ const i=name.lastIndexOf('.'); return i>=0?name.slice(i).toLowerCase():''; }

async function validateCommunityPac(blob, codec, name) {
  const profile=COMMUNITY_PROFILES.find(p=>p.name===codec);
  if(!profile) throw new Error(`Unknown Android14 profile: ${codec}`);
  if(blob.size<18) throw new Error(`Truncated Android14 PAC: ${name}`);
  const first=new DataView(await blob.slice(0,2).arrayBuffer());
  const count=(first.getUint16(0,true)^profile.count)>>>0;
  const base=2+count*16;
  if(!count || base>blob.size || base>MAX_ENTRY_SIZE) throw new Error(`Unsupported Android14 PAC codec: ${name}`);
  const bytes=new Uint8Array(await blob.slice(0,base).arrayBuffer());
  const view=new DataView(bytes.buffer,bytes.byteOffset,bytes.byteLength); let found=false;
  const types=new Set(profile.types.map(x=>x>>>0));
  for(let i=0;i<count;i++){
    const p=2+i*16;
    const offset=(u32(view,p)^profile.offset^i)>>>0, size=(u32(view,p+4)^profile.size^i)>>>0;
    let key=(((bytes[p+8]<<24)>>>0)|(bytes[p+9]<<16)|(bytes[p+10]<<8)|bytes[p+11])>>>0; key=(key^i)>>>0;
    if(types.has(key)) found=true;
    if(base+offset>blob.size || size>blob.size-base-offset) throw new Error(`Android14 PAC entry is out of bounds: ${name}`);
  }
  if(!found) throw new Error(`Unknown Android14 PAC codec: ${name}`);
}

function dosDateTime(date=new Date()){
  const year=Math.max(1980,date.getFullYear());
  const time=((date.getHours()&31)<<11)|((date.getMinutes()&63)<<5)|((Math.floor(date.getSeconds()/2))&31);
  const day=((year-1980)&127)<<9|((date.getMonth()+1)&15)<<5|(date.getDate()&31);
  return {time,day};
}

class StoredZipBuilder {
  constructor(){this.locals=[];this.centrals=[];this.offset=0;this.count=0;this.paths=new Set();}
  async add(path, blob, crc=null){
    if(this.paths.has(path)) throw new Error(`Duplicate package path: ${path}`); this.paths.add(path);
    const name=textEncoder.encode(path); if(name.length>0xffff) throw new Error(`ZIP path too long: ${path}`);
    if(blob.size>ZIP32_MAX) throw new Error(`ZIP32 file too large: ${path}`);
    if(crc===null) crc=await crc32Blob(blob);
    const {time,day}=dosDateTime();
    const local=new Uint8Array(30+name.length), lv=new DataView(local.buffer);
    put32(lv,0,0x04034b50);put16(lv,4,20);put16(lv,6,0x0800);put16(lv,8,0);put16(lv,10,time);put16(lv,12,day);put32(lv,14,crc);put32(lv,18,blob.size);put32(lv,22,blob.size);put16(lv,26,name.length);put16(lv,28,0);local.set(name,30);
    const central=new Uint8Array(46+name.length),cv=new DataView(central.buffer);
    put32(cv,0,0x02014b50);put16(cv,4,0x0314);put16(cv,6,20);put16(cv,8,0x0800);put16(cv,10,0);put16(cv,12,time);put16(cv,14,day);put32(cv,16,crc);put32(cv,20,blob.size);put32(cv,24,blob.size);put16(cv,28,name.length);put16(cv,30,0);put16(cv,32,0);put16(cv,34,0);put16(cv,36,0);put32(cv,38,0);put32(cv,42,this.offset);central.set(name,46);
    this.locals.push(local,blob);this.centrals.push(central);this.offset+=local.length+blob.size;this.count++;
    if(this.offset>ZIP32_MAX) throw new Error('Output package exceeds ZIP32 size limit.');
  }
  close(){
    if(this.count>0xffff) throw new Error('Too many files for ZIP32 output.');
    const centralSize=this.centrals.reduce((n,x)=>n+x.length,0); if(this.offset+centralSize>ZIP32_MAX) throw new Error('Output package exceeds ZIP32 size limit.');
    const end=new Uint8Array(22),ev=new DataView(end.buffer);put32(ev,0,0x06054b50);put16(ev,4,0);put16(ev,6,0);put16(ev,8,this.count);put16(ev,10,this.count);put32(ev,12,centralSize);put32(ev,16,this.offset);put16(ev,20,0);
    return new Blob([...this.locals,...this.centrals,end],{type:'application/zip'});
  }
}

function jsonBlob(value){return new Blob([JSON.stringify(value,null,2)+'\n'],{type:'application/json;charset=utf-8'});}
function textBlob(text){return new Blob([text],{type:'text/plain;charset=utf-8'});}
function sourceStem(filename){return filename.replace(/\.apk$/i,'');}

async function addPackageFile(ctx,path,blob,knownSha=null,knownCrc=null){
  const sha=knownSha||await sha256Blob(blob), crc=knownCrc===null?await crc32Blob(blob):knownCrc;
  ctx.packageFiles.push({path,blob,sha,crc});
  return {sha,crc};
}

export async function extractApks(files, {onProgress=()=>{}}={}) {
  const selected=[...files]; if(!selected.length) throw new Error('Choose at least one APK.');
  const usedProfiles=new Set(), ctx={packageFiles:[]}, reports=[];
  let apkIndex=0;
  for(const apk of selected){
    apkIndex++;
    if(!/\.apk$/i.test(apk.name)) throw new Error(`Not an APK file: ${apk.name}`);
    onProgress({phase:'hashing',apk:apk.name,apkIndex,totalApks:selected.length,percent:2,message:'Calculating APK SHA-256...'});
    const apkHash=await sha256Blob(apk);
    onProgress({phase:'reading',apk:apk.name,apkIndex,totalApks:selected.length,percent:7,message:'Reading APK directory...'});
    const zip=await parseZip(apk);
    const raw=zip.entries.filter(e=>!e.directory&&e.name.startsWith('res/raw/'));
    const assets=zip.entries.filter(e=>!e.directory&&e.name.startsWith('assets/'));
    const rawBytes=raw.reduce((n,e)=>n+e.size,0), assetBytes=assets.reduce((n,e)=>n+e.size,0);
    if(rawBytes>0&&assetBytes>0) throw new Error(`Ambiguous APK: non-empty game data exists in both res/raw and assets: ${apk.name}`);
    let layout,prefix,communityCodec=null,dynamicCodec=null,sourceEntries;
    if(rawBytes>0){layout='raw';prefix='res/raw/';sourceEntries=raw;}
    else if(assetBytes>0){
      layout='assets';prefix='assets/';sourceEntries=assets;
      communityCodec=detectCommunityProfile(assets.map(e=>e.name.slice(7)));
      if(communityCodec)layout='community14';
      else if(!assets.some(e=>e.name==='assets/common.pac')){
        // Unknown PRIVATE loader: recover profile from its DEX without running Android code.
        const dex=zip.entries.find(e=>e.name==='classes.dex'&&!e.directory);
        if(!dex||dex.size>16*1024*1024)throw Error('Unrecognized protected mod: no safe classes.dex loader');
        const loaded=await entryDataBlob(zip,dex);
        dynamicCodec=discoverPrivateCodec(new Uint8Array(await loaded.blob.arrayBuffer()));
        layout='community14-dynamic';
      }
    }
    else throw new Error(`The APK contains no usable game data in res/raw or assets: ${apk.name}`);

    let profile=`profiles/${sanitizeProfileName(sourceStem(apk.name))}`, original=profile, suffix=2;
    while(usedProfiles.has(profile.toLowerCase())) profile=`${original}_${suffix++}`;
    usedProfiles.add(profile.toLowerCase());
    const outputRoot=`data/DBTapBattle/${profile}/`;
    const items=[], names=new Map(); let bundledSave=false,total=0;
    for(const entry of zip.entries){
      if(!entry.name.startsWith(prefix))continue;
      let name=entry.name.slice(prefix.length);
      if(entry.directory){if(name)assertSafeName(name.replace(/\/$/,''));continue;}
      assertSafeName(name);
      if(layout==='community14')name=canonicalName(name,communityCodec);
      else if(dynamicCodec)name=canonicalPrivateName(name,dynamicCodec);
      const key=name.toLowerCase();if(names.has(key))throw new Error(`Duplicate name after normalization: ${name}`);
      const mode=(entry.externalAttrs>>>16)&0xf000;if(mode!==0&&mode!==0x8000)throw new Error(`Non-regular ZIP entry: ${name}`);
      if((entry.flags&1)!==0||![0,8].includes(entry.method))throw new Error(`Encrypted entry or unsupported compression: ${name}`);
      if(entry.size>MAX_ENTRY_SIZE)throw new Error(`Data exceeds 64 MiB per-file limit: ${name}`);
      if(key==='save.bin'){if(bundledSave)throw new Error('The selected APK layout contains more than one save.bin.');bundledSave=true;continue;}
      names.set(key,name);total+=entry.size;if(total>MAX_DATA_SIZE)throw new Error('Data exceeds 512 MiB per APK limit.');items.push({entry,name,originalName:entry.name.slice(prefix.length)});
    }
    for(const key of names.keys()){
      const parts=key.split('/');for(let i=1;i<parts.length;i++)if(names.has(parts.slice(0,i).join('/')))throw new Error(`File/directory collision: ${key}`);
    }
    if(!names.has('common.pac'))throw new Error(`common.pac was not found; this does not look like a complete Tap Battle APK: ${apk.name}`);

    if(dynamicCodec){
      // Every profile gets a bounded, data-only sidecar. The Vita will load it
      // before opening any PAC. Never transfer the DEX/native libraries.
      await addPackageFile(ctx,outputRoot+'dbtb_codec.json',jsonBlob(codecSidecar(dynamicCodec)));
    }
    const filesMeta=[], renamed=[], unknown=[]; let pos=0;
    for(const item of items){
      pos++;onProgress({phase:'extracting',apk:apk.name,apkIndex,totalApks:selected.length,percent:10+Math.floor(78*pos/Math.max(items.length,1)),message:`Extracting ${item.name}`,file:item.name,fileIndex:pos,totalFiles:items.length});
      const extracted=await entryDataBlob(zip,item.entry);
      if(layout==='community14'&&item.name.endsWith('.pac'))await validateCommunityPac(extracted.blob,communityCodec,item.name);
      else if(dynamicCodec&&item.name.endsWith('.pac'))await validatePrivatePac(extracted.blob,dynamicCodec,item.name);
      const sha=await sha256Blob(extracted.blob);
      await addPackageFile(ctx,outputRoot+item.name,extracted.blob,sha,extracted.crc);
      filesMeta.push({name:item.name,size:item.entry.size,sha256:sha,apk_path:item.entry.name});
      if(item.name!==item.originalName)renamed.push({apk_path:item.entry.name,name:item.name});
      if(!KNOWN_EXTENSIONS.has(extension(item.name)))unknown.push(item.name);
    }
    const canonicalSet=new Set([...names.values()]);
    const roster=characterInventory(canonicalSet);
    const notExtracted=zip.entries.filter(e=>!e.directory&&!e.name.startsWith(prefix)).map(e=>e.name);
    const manifest={format:4,tool:'DBTapBattle Web Extractor 1.0',runtime_contract:'profiles-v1',source_layout:layout,pac_codec:dynamicCodec?'dragontap-private-v1':layout==='community14'?communityCodec:'original-or-unknown',payloads_unchanged:true,standalone_profile:true,requires_game_directory:false,renamed_files:renamed,source_apk:apk.name,source_apk_sha256:apkHash,file_count:filesMeta.length,files:filesMeta,unknown_files:unknown,unknown_raw_files:layout==='raw'?unknown:[],not_extracted:notExtracted,vita_profile:profile,character_count:roster.count,character_indices:roster.completeIndices,character_runtime_compatible:roster.runtimeCompatible,incomplete_character_indices:roster.incomplete,character_indices_after_gap:roster.laterAfterGap,unsupported_character_files:roster.unsupported,save_policy:'per-profile-vpk-seed',bundled_save:bundledSave,profile_save_installed:false};
    await addPackageFile(ctx,outputRoot+'dbtb_manifest.json',jsonBlob(manifest));
    reports.push({apk:apk.name,profile,files:filesMeta.length,source_sha256:apkHash,character_count:roster.count,character_indices:roster.completeIndices,character_runtime_compatible:roster.runtimeCompatible,bundled_save:bundledSave,layout,pac_codec:manifest.pac_codec});
    onProgress({phase:'profile-ready',apk:apk.name,apkIndex,totalApks:selected.length,percent:90,message:`Profile ready: ${profile}`});
  }

  const guideLines=['DRAGON BALL TAP BATTLE DATA FOR PS VITA','', 'Generated locally in your browser. The APK was not uploaded to a server.','', 'Copy the data folder from THIS ZIP to the ux0: root using VitaShell/FTP/USB or another file-transfer method.','The final path must be ux0:data/DBTapBattle/profiles/<Profile>/.','Install Dragon Ball Tap Battle PS Vita v1.0 (TITLE_ID DBTB01178) separately. This ZIP contains data only.','','EXTRACTED PROFILES:'];
  for(const r of reports){const label=r.profile.slice(r.profile.lastIndexOf('/')+1);guideLines.push(`- ${r.apk}: ux0:data/DBTapBattle/${r.profile}/ -> select ${label} in the VPK.`);guideLines.push(`  Characters detected: ${r.character_count} ${r.character_indices}`);if(!r.character_runtime_compatible)guideLines.push('  WARNING: roster does not fully satisfy the current profiles-v1 contract; check dbtb_manifest.json.');if(r.bundled_save)guideLines.push('  The APK contained save.bin, but it was not installed. The VPK creates an independent profile save.');}
  guideLines.push('', 'Every APK is stored as an independent profile under profiles/, using the APK filename as the profile folder.','There is no separate game/ or mods/ layout and there is no cross-profile resource fallback.','To change the selector name, rename the extracted profile folder.','','PAC/media payloads are preserved byte-for-byte. Protected Android14-family profiles keep PAC bytes intact; unknown PRIVATE variants use DEX-derived aliases and per-profile dbtb_codec.json.','ZIP CRC, file sizes and SHA-256 hashes are verified. dbtb_manifest.json records provenance.','APK DEX/classes/native libraries are not copied as Vita gameplay code. Mods changing Android code may need Vita-side compatibility work.');
  await addPackageFile(ctx,'LEEME_COPIAR_A_VITA.txt',textBlob(guideLines.join('\n')+'\n'));
  await addPackageFile(ctx,'RESULTADO.json',jsonBlob({tool_version:'web-1.0',runtime_contract:'profiles-v1',runtime_root:'ux0:data/DBTapBattle/profiles/',standalone_profiles:true,verified:true,processing:'local-browser',title_id:'DBTB01178',profiles:reports}));
  const sums=ctx.packageFiles.slice().sort((a,b)=>a.path.localeCompare(b.path)).map(f=>`${f.sha}  ${f.path}`).join('\n')+'\n';
  await addPackageFile(ctx,'SHA256SUMS.txt',textBlob(sums));

  onProgress({phase:'packing',percent:94,message:'Building Vita-ready ZIP...'});
  const builder=new StoredZipBuilder();
  let i=0;
  for(const f of ctx.packageFiles){i++;await builder.add(f.path,f.blob,f.crc);onProgress({phase:'packing',percent:94+Math.floor(5*i/ctx.packageFiles.length),message:`Packing ${f.path}`,fileIndex:i,totalFiles:ctx.packageFiles.length});}
  const output=builder.close();
  const base=reports.length===1?sanitizeProfileName(sourceStem(reports[0].apk)):'Multiple_Profiles';
  onProgress({phase:'done',percent:100,message:'DATA READY FOR PS VITA'});
  return {blob:output,filename:`DBTapBattle-${base}-Vita.zip`,reports,packageSize:output.size,files:ctx.packageFiles.length};
}

export const LIMITS={MAX_APK_SIZE,MAX_ENTRY_SIZE,MAX_DATA_SIZE,MAX_ENTRIES};
