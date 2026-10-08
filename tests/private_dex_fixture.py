"""Synthetic DragonTap PRIVATE DEX+PAC corpus for real browser/Windows checks.
This fixture contains no game assets and is unrelated to protected commercial APK data.
"""
import struct
import zipfile
from pathlib import Path

ALIASES={'common':'4AF4','select0':'49D9','back':'1C2B','bobj':'71DD',
 'char':'36D7','chardemo':'CE4E','charf':'A540','effect':'49B9',
 'demo_00':'68ED','demo_08':'7CEF','font00':'D635',
 'card_preview':'4291','gamedata':'259D','text00':'2D2B','card':'AE9B'}
LABELS=('common','select0','back','back','bobj','bobj','char','chardemo',
 'charf','effect','demo_00','demo_08','font00','card_preview','gamedata','text00','card')
KEYS={'count_xor':24318,'offset_xor':2346498164,'size_xor':1511186348,
 'image_width_xor':3304,'image_height_xor':48805,'table_count_xor':14952,
 'table_position_xor':3392584592,'table_width_xor':5971,'table_height_xor':39614}
TYPES={'act':882113049,'bin':793741694,'cnv':1846986893,
 'dac':2966949568,'rgba':3328993045,'spr':3407625201,'wav':151024925}


def leb(v):
    b=bytearray()
    while True:
        x=v&127;v>>=7;b.append(x|(128 if v else 0))
        if not v:return bytes(b)


def dex():
    strings=list(dict.fromkeys(ALIASES[k] for k in LABELS))+['<clinit>','Lext/o;']
    so=0x70;to=so+4*len(strings);mo=to+4;co=mo+8
    b=bytearray(co+32)
    for i,s in enumerate(strings):
        struct.pack_into('<I',b,so+4*i,len(b))
        b.extend(leb(len(s))+s.encode('ascii')+b'\0')
    while len(b)%4:b.append(0)
    code_offset=len(b)
    words=[]
    for key in LABELS:words.extend((0x1a,strings.index(ALIASES[key]),0x4d,0))
    mask=0x4b4b4b4b
    values=[TYPES[key]^mask for key in ('act','bin','cnv','dac','rgba','spr','wav')]
    values.extend((KEYS['table_position_xor'],KEYS['offset_xor'],KEYS['size_xor'],
      mask,KEYS['count_xor'],KEYS['image_width_xor'],KEYS['image_height_xor'],
      KEYS['table_count_xor'],KEYS['table_width_xor'],KEYS['table_height_xor']))
    for index,value in enumerate(values):
        words.extend((0x14,value&65535,value>>16,0x67,100+index))
    b.extend(struct.pack('<HHHHII',2,0,0,0,0,len(words)))
    b.extend(struct.pack('<'+'H'*len(words),*words))
    data_offset=len(b)
    b.extend(leb(0)+leb(0)+leb(1)+leb(0)+leb(0)+leb(8)+leb(code_offset))
    struct.pack_into('<II',b,0x38,len(strings),so)
    struct.pack_into('<II',b,0x40,1,to)
    struct.pack_into('<II',b,0x58,1,mo)
    struct.pack_into('<II',b,0x60,1,co)
    struct.pack_into('<I',b,to,strings.index('Lext/o;'))
    struct.pack_into('<HHI',b,mo,0,0,strings.index('<clinit>'))
    struct.pack_into('<I',b,co+24,data_offset)
    struct.pack_into('<II',b,0x20,len(b),0x70)
    b[:8]=b'dex\n035\0'
    return bytes(b)


def pac():
    data=b'synthetic-protected-pac-payload'
    return (struct.pack('<HII',1^KEYS['count_xor'],KEYS['offset_xor'],
     len(data)^KEYS['size_xor'])+struct.pack('>I',TYPES['bin'])+b'\0'*4+data)


def make_apk(filename,corrupt=False):
    filename=Path(filename)
    file=pac()
    if corrupt:
        file=file[:5]
    with zipfile.ZipFile(filename,'w',zipfile.ZIP_STORED) as z:
        z.writestr('classes.dex',dex())
        for stem in ('4AF4','49D9','259D','2D2B','36D700','CE4E00','A5400000'):
            z.writestr('assets/'+stem+'.pac',file)
    return filename


if __name__=='__main__':
    import sys
    make_apk(sys.argv[1],len(sys.argv)>2 and sys.argv[2]=='corrupt')
