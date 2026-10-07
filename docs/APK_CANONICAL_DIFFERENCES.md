# Matriz de diferencias canónicas entre APK

Fecha: 2026-10-06.

Esta página compara los seis APK auditados **después de resolver su ubicación y
alias al nombre lógico que solicita el juego**. Es complementaria a
[APK_TECHNICAL_REFERENCE](APK_TECHNICAL_REFERENCE.md): aquí se responde
exactamente qué archivos existen, faltan o cambian entre perfiles sin confundir
`res/raw/`, `assets/` ni los nombres ofuscados.

Las comparaciones "idéntico/diferente" de esta página son SHA-256 de los bytes
del archivo completo salvo cuando se indique explícitamente **semántico**.
Dos PAC protegidos con distinto perfil pueden ser byte-diferentes aunque parte
de su contenido decodificado sea idéntico.

## Resumen

| Comparación | Nombres lógicos comunes | Byte-idénticos | Byte-distintos | Solo A | Solo B |
|---|---:|---:|---:|---:|---:|
| Original → Gen | 57 | 40 | 17 | 0 | 90 |
| Original → Android14 | 55 | 37 | 18 | 2 | 89 |
| Original → Español | 55 | 37 | 18 | 2 | 89 |
| Original → Invasion | 55 | 30 | 25 | 2 | 122 |
| Gen → Android14 | 144 | 38 | 106 | 3 | 0 |
| Gen → Español | 144 | 38 | 106 | 3 | 0 |
| Gen → Invasion | 144 | 31 | 113 | 3 | 33 |
| Android14 → Español | 144 | 38 | 106 | 0 | 0 |
| Android14 → Invasion | 144 | 31 | 113 | 0 | 33 |
| Español → Invasion | 144 | 31 | 113 | 0 | 33 |
| Original → Zuper/SamuGamerYT | 57 | 23 | 34 | 0 | 327 |
| Gen → Zuper/SamuGamerYT | 147 | 117 | 30 | 0 | 237 |

## Original → Gen

Los 57 recursos lógicos del APK original existen también en Gen.

**40 son byte-idénticos:**

- `bgm_00..16.ogg` (17);
- `se_00..18.ogg` (19);
- `demo_08.pac`;
- `effect.pac`;
- `font00.pac`;
- `mk.bin`.

**17 cambian:**

```text
back00.pac back01.pac back02.pac back03.pac
bobj00.pac bobj01.pac bobj02.pac bobj03.pac bobj04.pac
card000.pac card_preview.pac common.pac demo_00.pac
gamedata.pac loading.png select0.pac text00.pac
```

Gen añade **90** archivos lógicos:

- `card001..050.pac` = 50;
- `char00..12.pac` = 13;
- `chardemo00..12.pac` = 13;
- `charf0000..0012.pac` = 13;
- `save.bin` = 1.

No elimina ningún nombre lógico del dataset original.

## Original → Android14 / Español

Ambos perfiles protegidos tienen la misma cobertura lógica respecto al original.

Comunes: 55. Byte-idénticos: **37**, exactamente los 36 audios exteriores
(`bgm_00..16` + `se_00..18`) y `mk.bin`.

Los **18 nombres comunes byte-distintos** son:

```text
back00.pac back01.pac back02.pac back03.pac
bobj01.pac bobj02.pac bobj03.pac bobj04.pac
card000.pac card_preview.pac common.pac
demo_00.pac demo_08.pac effect.pac
gamedata.pac loading.png select0.pac text00.pac
```

Solo Original:

- `bobj00.pac`;
- `font00.pac`.

El perfil protegido añade **89** nombres que el APK original suministrado no trae:

- `card001..050` = 50;
- 13 tripletes `char/chardemo/charf` = 39.

La ausencia de `bobj00` y font no debe resolverse fabricando archivos: el modelo
Vita correcto es overlay + fallback a un base compatible.

## Original → Invasion Beta 3

Comunes: 55. Byte-idénticos: **30**:

- BGM sin cambiar: `00,01,02,08,09,10,11,12,13,16` = 10;
- todos los `se_00..18` = 19;
- `mk.bin` = 1.

Los 25 comunes distintos son los 18 recursos PAC/imagen que ya cambian en la
familia protegida, más los siete BGM de Invasion:

```text
bgm_03 bgm_04 bgm_05 bgm_06 bgm_07 bgm_14 bgm_15
back00..03
bobj01..04
card000 card_preview common demo_00 demo_08 effect
gamedata loading.png select0 text00
```

Solo Original: `bobj00.pac`, `font00.pac`.

Invasion añade **122 nombres** frente al APK original suministrado:

- `card001..050` = 50;
- `char00..21` = 22;
- `chardemo00..21` = 22;
- `charf0000..0021` = 22;
- `back04..06` = 3;
- `bobj05..07` = 3.

## Gen → Android14 / Español

Hay 144 nombres lógicos comunes.

**38 son byte-idénticos:**

- 36 audios exteriores;
- `loading.png`;
- `mk.bin`.

Los **106 PAC comunes** son byte-distintos. Esto era esperable incluso cuando
un asset visual es equivalente, porque Gen usa PAC ordinario y los protegidos
usan directorios/payloads diferentes.

Solo Gen:

- `bobj00.pac`;
- `font00.pac`;
- `save.bin`.

Android14/Español no añaden ningún nombre lógico que Gen no tenga.

No usar este "106 distintos" como evidencia de 106 cambios artísticos o de
gameplay; para eso se necesita la comparación semántica de imágenes/PCM/tablas.

## Gen → Invasion

Comunes: 144. Byte-idénticos: **31**:

- los 29 audios exteriores de Invasion que no cambian;
- `loading.png`;
- `mk.bin`.

Byte-distintos: **113** = los 106 PAC compartidos + los siete BGM cambiados.

Solo Gen: `bobj00.pac`, `font00.pac`, `save.bin`.

Solo Invasion: **33 PAC**:

```text
back04.pac back05.pac back06.pac
bobj05.pac bobj06.pac bobj07.pac
char13.pac .. char21.pac
chardemo13.pac .. chardemo21.pac
charf0013.pac .. charf0021.pac
```

## Android14 → Español

Los 144 nombres lógicos son comunes.

- 38 archivos son byte-idénticos: 36 audios + `loading.png` + `mk.bin`.
- los 106 PAC son byte-distintos porque el perfil XOR/nombres y parte del
  contenido cambian.
- DEX: 596 métodos con código comunes; 585 instrucciones idénticas y 11 distintas.
- semántica PAC confirmada: 7/106 PAC conservan el digest completo de
  imagen/PCM/tablas; 99 cambian al menos un componente normalizado.
- por componente top-level: 304/361 imágenes iguales, 39/198 voces PCM iguales,
  5/70 tablas convertidas iguales.

Por eso "todos los PAC tienen hash distinto" **no** significa que todo el juego
fue reemplazado; el cambio de protección por sí solo cambia los bytes.

## Android14 → Invasion

Los 144 nombres Android14 existen en Invasion.

- 31 byte-idénticos: 29 audios sin cambiar + `loading.png` + `mk.bin`.
- 113 byte-distintos: 106 PAC + siete BGM.
- Invasion añade 33 PAC.
- DEX: 595 firmas de código comunes; 223 instrucciones idénticas, 372 distintas;
  una firma `Primary.<init>` cambia con el package.
- 105/106 PAC compartidos conservan la misma **secuencia top-level de tipos**;
  la excepción es `demo_00`, donde Invasion añade una RGBA.
- digest semántico completo: solo 6/106 PAC coinciden
  (`back00..03`, `card000`, `card_preview`).

## Español → Invasion

Esta es la relación de contenido más cercana de los perfiles protegidos.

A nivel de archivo exterior:

- 144 nombres comunes;
- 31 byte-idénticos = 29 audios sin cambiar + `loading.png` + `mk.bin`;
- 113 byte-distintos = 106 PAC + siete BGM;
- 33 PAC exclusivos de Invasion.

A nivel **semántico decodificado** de los 106 PAC compartidos:

- 81 PAC conservan el digest de componentes confirmados;
- 25 PAC cambian.

Los 25 son:

```text
card048 card049 card050
char00 char01 char02 char03 char04 char05 char06
char07 char08 char09 char10 char11 char12
chardemo08
charf0008 charf0011
demo_00 demo_08 effect gamedata select0 text00
```

Comparación por componente top-level:

- RGBA: 303 iguales / 58 diferentes;
- voz PCM: 195 iguales / 3 diferentes (las tres en `char11`);
- tablas convertidas: 52 iguales / 18 diferentes.

Los 372 cambios de instrucciones DEX respecto a Android14 aparecen también
Español→Invasion (595 firmas comunes, 223 idénticas / 372 distintas). Esto separa
con claridad dos hechos:

1. Invasion reutiliza gran parte del **contenido** del perfil Español;
2. Invasion incorpora a la vez una modificación extensa de **código Dalvik**.

No atribuir autoría ni portar los 372 métodos automáticamente.

## Audio: igualdad de nombre no implica igualdad de codec

Original, Gen, Android14 y Español comparten byte a byte los 36 audios y todos
son Vorbis 44.1 kHz.

Invasion conserva 29. Los siete restantes mantienen extensión `.ogg` pero son:

- MP3: `bgm_03`, `06`, `07`, `14`, `15`;
- AAC/M4A: `bgm_04`, `05`.

Esta diferencia debe resolverse en la frontera de audio/importación Vita. No se
debe modificar `SoundEffect`, los estados del juego ni los nombres lógicos que
solicita el motor para "arreglar" el mod.

## Referencias exactas

- [Referencia técnica completa](APK_TECHNICAL_REFERENCE.md)
- [Android14](ANDROID14_APK.md)
- [Español](SPANISH_ANDROID14_APK.md)
- [Invasion Beta 3](INVASION_BETA3_APK.md)
- [Gen](ORIGINAL_PLUS_CHARACTERS_APK.md)
- [Evidencia machine-readable](evidence/apk_deep_structure_2026-10-06.json)

Los conteos de esta página se regeneraron directamente de los cinco APK
suministrados el 2026-10-06; no provienen de nombres inferidos manualmente.

## Gen → Zuper/SamuGamerYT

Esta es la comparación más importante para el nuevo mod porque ambos comparten
**exactamente el mismo `classes.dex` y AndroidManifest.xml**.

- 147 nombres de assets Gen están presentes también en Zuper/SamuGamerYT.
- **117 son byte-idénticos**.
- **30 cambian**.
- Gen no tiene archivos exclusivos frente al mod.
- Zuper/SamuGamerYT añade **237 PAC**, exactamente 79 tripletes nuevos de personaje.

Los 30 cambios comunes son 15 BGM y 15 PAC:

```text
bgm_00 bgm_01 bgm_02 bgm_03 bgm_04 bgm_05 bgm_06 bgm_07
bgm_08 bgm_09 bgm_10 bgm_11 bgm_14 bgm_15 bgm_16

bobj04
char00 char10 char11
chardemo00 chardemo10
charf0000 charf0010
common demo_00 demo_08 effect gamedata select0 text00
```

Los 237 exclusivos son:

- `char13..91` = 79;
- `chardemo13..91` = 79;
- `charf0013..0091` = 79.

No añade cartas, backgrounds ni bobj nuevos. Las 51 cartas, `back00..03` y la
cantidad `bobj00..04` permanecen en el mismo rango lógico de Gen.

Dos PAC comunes cambian además de layout exterior:

- `char10`: 34 → 38 entries, añadiendo cuatro PNG;
- `charf0010`: 5 entries `PNG×3,CNV,DAC` → 2 entries `CNV,DAC`.

El resto de los PAC comunes modificados conserva la secuencia de tipos exterior.

La expansión a 92 personajes con DEX Gen intacto es evidencia de que este mod
pretende ser data-driven. La prueba física 00.32 confirmó que Vita expone los 92
personajes; la cobertura exhaustiva de cada personaje/pelea sigue siendo aparte.
Consulta [la auditoría específica](DRAGONBALL_ZUPER_SAMUGAMERYT_APK.md).

## Original → Zuper/SamuGamerYT

Los 57 nombres del APK original suministrado aparecen en el mod.

- 23 son byte-idénticos: `bgm_12`, `bgm_13`, los 19 `se_00..18`,
  `font00.pac` y `mk.bin`.
- 34 cambian.
- el mod añade 327 nombres lógicos sobre esos 57.

Esto separa claramente el rol del mod: conserva partes del baseline, adopta la
arquitectura offline de Gen y expande sobre ella.

## Estado de hardware 00.33

Las diferencias canónicas anteriores siguen describiendo los APK, pero ya existe
evidencia de runtime adicional: Samu muestra sus 92 personajes en Vita y el perfil
Invasion supera varias peleas consecutivas en 00.33 después de corregir una copia
nativa duplicada del PAC protegido normalizado. Ese arreglo no cambia ninguna de
las diferencias de APK documentadas aquí.
