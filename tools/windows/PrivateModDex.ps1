#requires -Version 5.1
# Data-only Dalvik inspection for DragonTap_Util PRIVATE generated mods.
# Used only for unknown protected aliases; no Android code is executed.
Add-Type -TypeDefinition @'
using System;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using System.Collections.Generic;
public sealed class PrivateDexProfile {
 public Dictionary<string,string> Aliases=new Dictionary<string,string>(StringComparer.Ordinal);
 public Dictionary<string,uint> Keys=new Dictionary<string,uint>(StringComparer.Ordinal);
 public string Canonical(string name) {
  if(!name.EndsWith(".pac",StringComparison.Ordinal)||name.IndexOf('/')>=0)return name;
  string stem=name.Substring(0,name.Length-4);
  string[] fixedNames={"common","select0","effect","demo_00","demo_08","font00","card_preview","gamedata","text00"};
  foreach(string key in fixedNames)if(stem==Aliases[key])return key+".pac";
  string[] types={"back","bobj","char","chardemo","charf","card"};
  int[] lengths={2,2,2,2,4,3};
  for(int i=0;i<types.Length;i++){string prefix=Aliases[types[i]];
   if(!stem.StartsWith(prefix,StringComparison.Ordinal))continue;
   string suffix=stem.Substring(prefix.Length);
   if(suffix.Length==lengths[i]&&Regex.IsMatch(suffix,@"^[0-9]+$"))return types[i]+suffix+".pac";
  }
  return name;
 }
 public bool ValidPac(byte[] b) {
  if(b.Length<18)return false;
  uint count=(uint)(BitConverter.ToUInt16(b,0)^Keys["count_xor"]);
  ulong basePos=2UL+16UL*count;
  if(count==0||count>8192||basePos>(ulong)b.Length)return false;
  bool recognized=false;
  for(int i=0;i<count;i++){int row=2+i*16;
   uint offset=BitConverter.ToUInt32(b,row)^Keys["offset_xor"]^(uint)i;
   uint size=BitConverter.ToUInt32(b,row+4)^Keys["size_xor"]^(uint)i;
   if(offset>(ulong)b.Length-basePos||size>(ulong)b.Length-basePos-offset)return false;
   uint typ=((uint)b[row+8]<<24)|((uint)b[row+9]<<16)|((uint)b[row+10]<<8)|b[row+11];typ^=(uint)i;
   bool known=false;
   foreach(string key in new string[]{"act","bin","cnv","dac","rgba","spr","wav"})
     if(typ==Keys["type_"+key]){known=true;break;}
   bool plt=b[row+8]==112&&b[row+9]==108&&b[row+10]==116&&b[row+11]==0;
   if(!known&&!plt)return false;
   recognized=true;
  }
  return recognized;
 }
}
public static class PrivateModDexReader {
 static void Need(byte[] b,long pos,long n){if(pos<0||n<0||pos+n>b.Length)throw new InvalidDataException("Invalid/truncated DEX");}
 static ushort U16(byte[] b,long p){Need(b,p,2);return BitConverter.ToUInt16(b,(int)p);}
 static uint U32(byte[] b,long p){Need(b,p,4);return BitConverter.ToUInt32(b,(int)p);}
 static long Leb(byte[] b,ref long p){long v=0;for(int i=0;i<5;i++){Need(b,p,1);int x=b[p++];v|=(long)(x&127)<<(i*7);if(x<128)return v;}throw new InvalidDataException("Invalid DEX ULEB");}
 static PrivateDexProfile ParseInitializer(ushort[] w,string[] strings) {
  var aliases=new List<string>();var assignments=new List<KeyValuePair<int,uint>>();
  for(int i=0;i<w.Length;i++){
   int op=w[i]&255;
   if(op==0x1a&&i+2<w.Length){int id=w[i+1];
    if(id<strings.Length&&Regex.IsMatch(strings[id],@"^[0-9A-F]{4}$")&&(w[i+2]&255)==0x4d){aliases.Add(strings[id]);i+=2;continue;}
   }
   if(op==0x13||op==0x14||op==0x15){int width=op==0x14?3:2;
    if(i+width+1<w.Length&&(w[i+width]&255)==0x67){
     uint val=op==0x14?((uint)w[i+1]|((uint)w[i+2]<<16)):op==0x15?((uint)w[i+1]<<16):(uint)w[i+1];
     assignments.Add(new KeyValuePair<int,uint>(w[i+width+1],val));i+=width+1;continue;
    }
   }
  }
  if(aliases.Count!=17||assignments.Count<17||aliases[2]!=aliases[3]||aliases[4]!=aliases[5])return null;
  var blocks=new List<uint[]>();
  for(int i=0;i<=assignments.Count-17;i++){
   bool ok=true;for(int j=1;j<17;j++)if(assignments[i+j-1].Key+1!=assignments[i+j].Key){ok=false;break;}
   if(ok){uint[] v=new uint[17];for(int j=0;j<17;j++)v[j]=assignments[i+j].Value;blocks.Add(v);}
  }
  if(blocks.Count!=1)return null;
  uint[] values=blocks[0];var p=new PrivateDexProfile();
  string[] labels={"common","select0","back","back","bobj","bobj","char","chardemo","charf","effect","demo_00","demo_08","font00","card_preview","gamedata","text00","card"};
  for(int i=0;i<17;i++)p.Aliases[labels[i]]=aliases[i];
  if(new HashSet<string>(p.Aliases.Values).Count!=p.Aliases.Count)return null;
  string[] labelsType={"act","bin","cnv","dac","rgba","spr","wav"};
  for(int i=0;i<7;i++)p.Keys["type_"+labelsType[i]]=values[i]^values[10];
  p.Keys["count_xor"]=values[11];p.Keys["offset_xor"]=values[8];p.Keys["size_xor"]=values[9];
  p.Keys["image_width_xor"]=values[12];p.Keys["image_height_xor"]=values[13];
  p.Keys["table_count_xor"]=values[14];p.Keys["table_position_xor"]=values[7];
  p.Keys["table_width_xor"]=values[15];p.Keys["table_height_xor"]=values[16];p.Keys["wav_size_xor"]=values[11];
  foreach(string key in new string[]{"count_xor","image_width_xor","image_height_xor","table_count_xor","table_width_xor","table_height_xor"})
   if(p.Keys[key]>65535)return null;
  if(new HashSet<uint>(new uint[]{p.Keys["type_act"],p.Keys["type_bin"],p.Keys["type_cnv"],p.Keys["type_dac"],p.Keys["type_rgba"],p.Keys["type_spr"],p.Keys["type_wav"]}).Count!=7)return null;
  return p;
 }
 public static PrivateDexProfile Read(byte[] b) {
  Need(b,0,112);if(b[0]!=100||b[1]!=101||b[2]!=120||b[3]!=10)throw new InvalidDataException("Not Dalvik DEX");
  uint nstr=U32(b,0x38),ostr=U32(b,0x3c),nt=U32(b,0x40),ot=U32(b,0x44),nm=U32(b,0x58),om=U32(b,0x5c),nc=U32(b,0x60),oc=U32(b,0x64);
  if(nstr>100000||nt>100000||nm>100000||nc>100000)throw new InvalidDataException("Excess DEX ids");
  Need(b,ostr,(long)nstr*4);Need(b,ot,(long)nt*4);Need(b,om,(long)nm*8);Need(b,oc,(long)nc*32);
  var strings=new string[nstr];for(int i=0;i<nstr;i++){long p=U32(b,(long)ostr+i*4);Leb(b,ref p);long stop=p;while(stop<b.Length&&stop-p<=65536&&b[stop]!=0)stop++;Need(b,stop,1);if(stop-p>65536)throw new InvalidDataException("Long DEX string");strings[i]=Encoding.UTF8.GetString(b,(int)p,(int)(stop-p));}
  var types=new string[nt];for(int i=0;i<nt;i++){uint id=U32(b,(long)ot+i*4);if(id>=nstr)throw new InvalidDataException("Bad DEX type");types[i]=strings[id];}
  var methodNames=new string[nm];for(int i=0;i<nm;i++){int type=U16(b,(long)om+i*8);uint name=U32(b,(long)om+i*8+4);if(type>=nt||name>=nstr)throw new InvalidDataException("Bad DEX method");methodNames[i]=strings[name];}
  var matches=new List<PrivateDexProfile>();
  for(int c=0;c<nc;c++){long p=U32(b,(long)oc+c*32+24);if(p==0)continue;
   long[] counts=new long[4];for(int i=0;i<4;i++){counts[i]=Leb(b,ref p);if(counts[i]>100000)throw new InvalidDataException("DEX member budget");}
   for(int group=0;group<2;group++)for(int j=0;j<counts[group];j++){Leb(b,ref p);Leb(b,ref p);}
   for(int group=2;group<4;group++){long mid=0;for(int j=0;j<counts[group];j++){mid+=Leb(b,ref p);Leb(b,ref p);long off=Leb(b,ref p);
    if(mid>=nm||off==0||methodNames[mid]!="<clinit>")continue;
    uint num=U32(b,off+12);if(num>65536)continue;Need(b,off+16,(long)num*2);
    ushort[] instructions=new ushort[num];for(int k=0;k<num;k++)instructions[k]=U16(b,off+16+k*2);
    var candidate=ParseInitializer(instructions,strings);if(candidate!=null)matches.Add(candidate);
   }}
  }
  if(matches.Count!=1)throw new InvalidDataException("Expected exactly one DragonTap PRIVATE initializer, found "+matches.Count);
  return matches[0];
 }
}
'@ -ErrorAction Stop

function Get-PrivateModProfile($Archive) {
    $dex=$Archive.GetEntry('classes.dex')
    if ($null -eq $dex -or $dex.Length -le 0 -or $dex.Length -gt 16MB) {
        throw 'Unknown protected mod: missing/big classes.dex'
    }
    $stream=$dex.Open()
    try {
        $buffer=New-Object IO.MemoryStream
        try { $stream.CopyTo($buffer); $bytes=$buffer.ToArray() }
        finally { $buffer.Dispose() }
    } finally { $stream.Dispose() }
    return [PrivateModDexReader]::Read($bytes)
}

function Get-PrivateCodecSidecar($Private) {
    $out=[ordered]@{ format=1; generator='dragontap-private-v1' }
    foreach ($k in @('count_xor','offset_xor','size_xor','image_width_xor','image_height_xor','table_count_xor','table_position_xor','table_width_xor','table_height_xor','wav_size_xor',
                      'type_act','type_bin','type_cnv','type_dac','type_rgba','type_spr','type_wav')) {
        $out[$k]=[uint32]$Private.Keys[$k]
    }
    return $out
}
