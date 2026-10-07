# DragonBallZuperSamuGamerYT.apk — referencia técnica completa

Auditoría directa: **2026-10-06**.

Fuente suministrada al proyecto:

- archivo: `DragonBallZuperSamuGamerYT.apk`
- APK SHA-256: `1771d71de25d664894dfb33b4a296ad6d30f5d897a34eb1ec14f3135496ec41d`
- tamaño: **388,634,969 bytes**
- entradas ZIP: **461**
- estado en este documento: **APK observado directamente**
- no se almacena ningún payload comercial en Git; solo metadatos, hashes y estructura.

Este APK es especialmente importante porque demuestra un caso de expansión
masiva de contenido usando el **mismo DEX de Gen**. No pertenece a la familia
protegida Android14/Español/Invasion.

## 1. Identidad Android y relación con Gen

### DEX

`classes.dex`:

- tamaño: **700,624 bytes**
- SHA-256:
  `cba71bc13b9d1281aa8180423be9d08db0deb0fc2f5ef6825cc11ba66a17b729`

Es **byte-idéntico al `classes.dex` de `gen.apk`**.

Por tanto, para este APK no hay evidencia de una modificación Dalvik adicional
respecto a Gen. Cualquier diferencia de contenido observada abajo viene de
resources/assets/empaquetado, no de un DEX nuevo.

Esto no significa automáticamente que todo el roster de 92 esté confirmado en
Vita. Sí demuestra que el APK fue construido para usar la misma lógica Java/Dalvik
que Gen con un dataset mucho mayor.

### AndroidManifest.xml

- tamaño: **4,852 bytes**
- SHA-256:
  `0d83c2e0880708eb24c61ec870a87c90a6d08c8b43dae000ce641896b193726e`
- **byte-idéntico a Gen**.

Por herencia exacta del manifiesto Gen:

- package: `com.namcobandaigames.dragonballtap.apk`
- versionName: `1.4`
- versionCode: `7`
- minSdkVersion: `9`
- launcher: `.dragonballtap`
- actividades Bluetooth/Smap heredadas
- billing heredado
- `debuggable=true`.

### Firma

Usa el mismo certificado Android genérico observado en Gen:

- serial: `936EACBE07F201DF`
- SHA-256 del certificado:
  `a40da80a59d170caa950cf15c18c454d47a39b26989d8b640ecd745ba71bf5dc`
- subject:
  `C=US, ST=California, L=Mountain View, O=Android, OU=Android, CN=Android, emailAddress=android@android.com`

El contenedor PKCS#7 `CERT.RSA` no tiene el mismo hash que Gen porque firma un
APK diferente; el certificado contenido sí es el mismo.

### resources.arsc e iconos

`resources.arsc` cambia respecto a Gen:

- Gen: 12,084 bytes, SHA-256
  `41109966c57934da90f7aef83350a3d3898d70f21f26130495f65cc173c118b8`
- este mod: 12,076 bytes, SHA-256
  `a45d396f099d67f89e1454598a070146b61b6be999d00c2e25fc5742581f57b9`

Las cadenas visibles principales inspeccionadas siguen incluyendo el mismo
package y el mismo nombre `tap battle`.

Los tres iconos launcher sí cambian:

| Ruta | Dimensión | Bytes | SHA-256 |
|---|---:|---:|---|
| `res/drawable-ldpi/icon.png` | 36×36 | 4,343 | `6430bf7794892e95c5f9396b38cf0b4750a3da1f2adbaf8a9e02271a8c140521` |
| `res/drawable-mdpi/icon.png` | 48×48 | 7,314 | `bf7907c70033fdbb3be6f3a841bceea010bcae2910392eb2a84b5b2c85c763e1` |
| `res/drawable-hdpi/icon.png` | 72×72 | 15,728 | `28cd2349c84b27abcaa6c43697e111e1534007f24964c0baf07018dc7c1d56cb` |

## 2. Layout ZIP / dataset

Entradas de datos:

- `assets/`: **384 archivos**
- `res/raw/`: **57 archivos**
- `lib/`: **0 archivos**

Los 57 `res/raw/` existen, pero **todos son stubs de cero bytes**, igual que en
Gen. Los datos reales están en `assets/`.

Distribución de `assets/`:

- **345 `.pac`**
- **36 archivos llamados `.ogg`**
- **2 `.bin`**
- **1 `.png`**

Los dos BIN son:

- `mk.bin`
- `save.bin`.

### Archivos compartidos con Gen

`loading.png`:

- 128×128
- 3,394 bytes
- SHA-256
  `92565342ed5d33ce05400d1f4cd9ada2159bd83f2f87e55a12f897c8a70c817b`
- byte-idéntico a Gen/Android14/Español/Invasion.

`mk.bin`:

- 392 bytes
- SHA-256
  `2caef8c445b71896f34d560e90be6e04d1911d6bf8010d73c84dd9288b30251f`
- byte-idéntico al resto de APK auditados.

`save.bin`:

- 12,906 bytes
- SHA-256
  `8b65f591ca2ba4724af55bfab4adf9e8a4260c7cd517f6bd93aa4c69a1239714`
- **byte-idéntico al save incluido en Gen**.

No usar este save para sobrescribir automáticamente progreso de Vita.

## 3. Inventario lógico

Este mod usa nombres canónicos ordinarios; no necesita aliases protegidos.

### Conjunto general

- `back00..03.pac`: **4**
- `bobj00..04.pac`: **5**
- `card000..050.pac`: **51**
- `char00..91.pac`: **92**
- `chardemo00..91.pac`: **92**
- `charf0000..0091.pac`: **92**
- `bgm_00..16.ogg`: **17**
- `se_00..18.ogg`: **19**
- `card_preview.pac`
- `common.pac`
- `demo_00.pac`
- `demo_08.pac`
- `effect.pac`
- `font00.pac`
- `gamedata.pac`
- `select0.pac`
- `text00.pac`
- `loading.png`
- `mk.bin`
- `save.bin`.

### Expansión de personajes

Hay **92 tripletes nominales completos**:

```text
char00..91
chardemo00..91
charf0000..0091
```

Eso son **276 PAC ligados a personajes**.

Respecto a Gen:

- Gen: 13 personajes (`00..12`)
- este mod: 92 (`00..91`)
- incremento: **79 personajes**
- archivos añadidos: **79 × 3 = 237 PAC**.

No añade nuevas cartas ni nuevos backgrounds/bobj: la expansión de cantidad está
concentrada en los personajes.

## 4. Comparación exacta contra Gen

Gen contiene 147 archivos en `assets/`. Los 147 nombres existen también aquí.

- nombres comunes: **147**
- byte-idénticos: **117**
- byte-distintos: **30**
- solo Gen: **0**
- solo este mod: **237**

Los 237 exclusivos son exactamente los tripletes de personajes `13..91`.

### Los 30 archivos comunes que cambian

15 BGM:

```text
bgm_00 bgm_01 bgm_02 bgm_03 bgm_04
bgm_05 bgm_06 bgm_07 bgm_08 bgm_09
bgm_10 bgm_11 bgm_14 bgm_15 bgm_16
```

15 PAC:

```text
bobj04.pac
char00.pac
char10.pac
char11.pac
chardemo00.pac
chardemo10.pac
charf0000.pac
charf0010.pac
common.pac
demo_00.pac
demo_08.pac
effect.pac
gamedata.pac
select0.pac
text00.pac
```

Por tanto permanecen byte-idénticos a Gen, entre otros:

- las **51 cartas**
- `back00..03`
- `bobj00..03`
- la mayoría de personajes 00..12
- todos los 19 SE
- `font00.pac`
- `loading.png`
- `mk.bin`
- `save.bin`.

### Cambios estructurales entre los PAC comunes modificados

La mayoría conserva exactamente su secuencia de tipos.

Excepciones:

- `char10.pac`
  - Gen: 34 entries, 16 PNG + BIN + CNV + DAC + 15 WAV
  - mod: **38 entries**, 20 PNG + BIN + CNV + DAC + 15 WAV
- `charf0010.pac`
  - Gen: 5 entries = PNG×3, CNV, DAC
  - mod: **2 entries = CNV, DAC**
  - no contiene imágenes.

Los demás PAC modificados conservan su layout exterior aunque cambien payloads.

## 5. PAC ordinario: validación completa

Los 345 PAC exteriores usan el formato ordinario, no XOR:

```text
u16_le count
count × {
    u32_le offset_from_data_base
    u32_le size
    char type[4]
    u32_le reserved
}
data_base = 2 + count * 16
```

Resultado de la auditoría:

- **345/345 PAC exteriores válidos**
- **0 directorios fuera de bounds**
- **6 SPR anidados válidos**
- total contenedores auditados: **351**
- máximo PNG observado: **512×512**
- no se observan texturas mayores de 512×512 en este mod.

### Conteos de tipos

Top-level:

- PNG: **1,994**
- WAV: **1,443**
- CNV: **343**
- DAC: **343**
- BIN: **148**
- SPR: **6**
- ACT: **4**
- PLT: **5**
- tipo `u`: **18**
- tipo `.pn`: **1**

Incluyendo SPR anidados:

- PNG: **2,023**
- BIN: **154**
- el resto conserva los conteos anteriores.

Total de entries top-level: **4,305**.

## 6. Contrato de los 92 charXX

Los **92/92 `charXX.pac` contienen exactamente un BIN convertido de 43
registros**.

Este es uno de los hallazgos más importantes: el mismo contrato de datos
observado en los 13 personajes Gen continúa hasta el personaje 91.

No todos tienen exactamente la misma cantidad de sprites/voz:

- entries mínimos observados: **31** (`char60`)
- entries máximos observados: **48** (`char35`, `char54`)
- PNG mínimos observados: **14** (`char66`)
- PNG máximos observados: **21** (`char03`, `char79`, `char83`)
- WAV mínimos observados: **10** (`char60`)
- WAV máximos observados: **19** (`char28`, `char69`).

La mayoría sigue el patrón:

```text
PNG... -> BIN -> CNV -> DAC -> WAV...
```

pero el parser no debe validar por un número fijo de imágenes o voces.

## 7. Tipos no estándar dentro de PAC

### Tipo `u`

Se observan **18 entries con type = `u`**.

Aparecen únicamente en:

- `char35.pac`: 8
- `char54.pac`: 8
- `chardemo35.pac`: 1
- `chardemo54.pac`: 1.

Los payloads son strings ASCII/UTF-8 de **76 bytes** con URLs HTTP bajo el host
`pm1.narvii.com`, terminadas en referencias `.jpg`.

En `char35` y `char54` se intercalan entre PNG:

```text
png, u, png, u, ... (8 pares)
```

La evidencia sugiere metadata/referencias de procedencia de imágenes, no un
recurso gráfico embebido. No asumir que el motor debe dereferenciar esas URLs.
No añadir red ni descarga remota al port sin evidencia de que el juego las usa.

`chardemo35` y `chardemo54` añaden una entry `u` al patrón ordinario
`CNV,DAC,PNG`.

### Tipo `.pn`

`char42.pac`, entry 9, tiene:

- type bytes: `2E 70 6E 00` → `.pn`
- tamaño: **28,635 bytes**
- payload magic: PNG válido (`89 50 4E 47 0D 0A 1A 0A`).

Es decir: la etiqueta está malformada/atípica, pero el contenido es un PNG real.

Esto debe quedar documentado como anomalía de fuente. El port no debe cambiar el
parser global a "todo desconocido que parezca PNG = PNG" sin una prueba que
demuestre que esa entry es consumida por el juego. Si se necesita soporte, debe
ser una adaptación mínima y basada en evidencia.

## 8. chardemoXX y charfXXXX

### chardemo

- 92 archivos
- 90 usan exactamente:
  `CNV, DAC, PNG`
- `chardemo35` y `chardemo54` usan:
  `CNV, DAC, PNG, u`.

Los 92 chardemo tienen hashes únicos.

### charf

Los 92 nombres existen, pero su contenido varía bastante.

Patrones:

- 65 archivos:
  `PNG, PNG, CNV, DAC`
- 20 archivos:
  `CNV, DAC`
- 4 archivos:
  `PNG, PNG, PNG, CNV, DAC`
- 1 archivo (`charf0052`):
  `PNG, CNV, DAC`
- 2 archivos:
  PAC vacío con count=0.

Los dos PAC vacíos son:

- `charf0020.pac`
- `charf0021.pac`

Ambos tienen tamaño **2 bytes** y son estructuralmente PAC válidos con
`count=0`.

Índices con solo `CNV,DAC`:

```text
10 15 16 17 18 19 27 28 29 44
45 46 47 55 75 79 80 87 88 89
```

Índices con 3 PNG:

```text
08 13 23 78
```

Esto significa que "triplete presente" no equivale a "charf con imágenes".
Los validadores deben separar:

1. existencia/parseabilidad del PAC;
2. contenido opcional/placeholder;
3. capacidad real del motor para usar ese índice.

Solo hay **36 hashes únicos de charf entre 92 archivos**; hay reutilización y
placeholders extensivos. En contraste, los 92 `charXX` y los 92
`chardemoXX` son todos byte-distintos entre sí.

## 9. Cartas, escenarios y tablas

### Cartas

- `card000..050`: 51
- mismas 51 cartas presentes en Gen
- las 51 son **byte-idénticas a Gen**.

Cada card mantiene el contrato BIN de 1 registro observado en Gen.

### Background / bobj

- `back00..03`: byte-idénticos a Gen.
- `bobj00..03`: byte-idénticos a Gen.
- `bobj04.pac`: cambia, pero conserva layout
  `PNG,PNG,CNV,DAC`.

No hay `back04+` ni `bobj05+`.

### GameData / selección

Cambian respecto a Gen:

- `gamedata.pac`
- `select0.pac`
- `text00.pac`
- `common.pac`.

Esto es coherente con una expansión de roster impulsada por datos.

`gamedata.pac` conserva exactamente:

- 86,373 bytes
- 2 entries
- `CNV,DAC`

pero cambia su hash respecto a Gen.

`text00.pac`:

- 26,963 bytes
- `CNV,DAC`
- la tabla DAC conserva **1 registro / 315 celdas**
- contiene texto español UTF-8 dentro del contenido lógico.

No confundir el DEX Gen intacto con contenido de idioma intacto: este APK combina
el código Gen con datos/textos modificados.

## 10. Audio exterior: extensión .ogg no es codec

Los 19 SE son byte-idénticos a Gen y siguen siendo Vorbis 44.1 kHz mono.

De los 17 BGM:

- solo `bgm_12.ogg` y `bgm_13.ogg` permanecen byte-idénticos al baseline
  Vorbis;
- los otros **15 cambian**.

Codec real por contenido:

| BGM | Codec real | Hz | ch | Duración aprox. | Bytes | SHA-256 prefix |
|---|---|---:|---:|---:|---:|---|
| 00 | MP3 | 44100 | 2 | 85.159 s | 1,487,358 | `bef751b48466` |
| 01 | MP3 | 44100 | 2 | 139.337 s | 2,333,810 | `4164ffca64c4` |
| 02 | MP3 | 44100 | 2 | 161.889 s | 2,590,598 | `e7a8e589b6f9` |
| 03 | MP3 | 44100 | 2 | 88.346 s | 1,546,843 | `770dd996d604` |
| 04 | MP3 | 44100 | 2 | 88.346 s | 1,546,843 | `770dd996d604` |
| 05 | MP3 | 44100 | 2 | 88.346 s | 1,546,843 | `770dd996d604` |
| 06 | MP3 | 44100 | 2 | 88.346 s | 1,546,843 | `770dd996d604` |
| 07 | MP3 | 44100 | 2 | 88.346 s | 1,546,843 | `770dd996d604` |
| 08 | MP3 | 44100 | 2 | 88.346 s | 1,546,843 | `770dd996d604` |
| 09 | AAC/M4A | 44100 | 2 | 5.250 s | 78,860 | `cc72739a23e1` |
| 10 | AAC/M4A | 44100 | 2 | 36.598 s | 598,959 | `4119ed102c82` |
| 11 | AAC/M4A | 44100 | 2 | 5.250 s | 78,860 | `cc72739a23e1` |
| 12 | Vorbis | 44100 | 2 | 2.262 s | 41,141 | `3d0a666fdceb` |
| 13 | Vorbis | 44100 | 2 | 5.462 s | 92,961 | `ef06166a77e5` |
| 14 | MP3 | 44100 | 2 | 88.346 s | 1,546,843 | `770dd996d604` |
| 15 | MP3 | 44100 | 2 | 88.346 s | 1,546,843 | `770dd996d604` |
| 16 | MP3 | 44100 | 2 | 85.159 s | 1,487,358 | `bef751b48466` |

Duplicados exactos dentro del mod:

- `bgm_03..08` y `bgm_14..15` son el mismo MP3 byte por byte.
- `bgm_00` = `bgm_16`.
- `bgm_09` = `bgm_11`.

Implicación Vita: el camino actual basado en libvorbisfile no cubre 15/17 BGM de
este mod. La adaptación correcta pertenece al servicio de audio/importación Vita,
no al motor original.

## 11. Qué demuestra este APK para la arquitectura

Este mod aporta una evidencia distinta a Invasion:

### Invasion

- 22 personajes
- DEX ampliamente modificado
- recursos protegidos
- Tier C claro.

### Zuper/SamuGamerYT

- **92 personajes**
- DEX **idéntico a Gen**
- PAC ordinarios
- nombres canónicos
- expansión casi totalmente data-driven.

Esto sugiere que la arquitectura Java de Gen puede consumir un dataset de roster
mucho mayor sin necesitar un DEX específico para cada personaje añadido.

Pero no debe convertirse en una afirmación automática de que el port Vita ya
soporta 92:

- el runtime Vita debe poder resolver los nombres 00..91;
- selección/UI/memoria/save deben probarse;
- algunos charf son placeholders o vacíos;
- hay tipos `u` y `.pn` no presentes en el corpus Gen normal;
- 15 BGM requieren codec no-Vorbis;
- una prueba física sigue siendo obligatoria.

## 12. Integración del gate Vita en 00.26

Antes de 00.26, `auditInstalledData()` solo inspeccionaba 31 posiciones
(`00..30`). Esa era una política del adaptador Vita, no un límite demostrado
del roster del juego.

La evidencia de este APK —DEX Gen byte-idéntico, 92 tripletes completos y
92/92 `charXX` con el mismo contrato BIN de 43 registros— permitió ampliar de
forma focalizada **la auditoría Vita**, no la lógica del motor. 00.26 valida el
espacio completo de IDs de dos dígitos:

```text
char00.pac .. char99.pac
chardemo00.pac .. chardemo99.pac
charf0000.pac .. charf0099.pac
```

Eso equivale a capacidad de auditoría para hasta 100 posiciones `00..99`.
El dataset Samu observado usa 92 (`00..91`). El gate sigue exigiendo mínimo
13 personajes, tripletes completos y secuencia contigua; una ausencia al final
es válida, pero un hueco seguido de índices posteriores es error.

Las regresiones sintéticas cubren 13, 22, 92 y 100 tripletes y también un bound
inválido >100. No se modificó `Utility.InttoString()`, `TCBManajer`, Game3,
la selección ni el combate. El límite de dos dígitos se conserva: un futuro
índice 100 requiere evidencia separada y no forma parte de este soporte.

Esto fue inicialmente **HOST/ADAPTER CONFIRMED**; desde la prueba física 00.32 el runtime muestra el roster completo de 92 personajes en Vita. La compatibilidad de cada personaje/pelea individual sigue siendo una matriz más amplia
personajes.

## 13. Compatibilidad esperada del extractor

Este APK ya usa nombres canónicos dentro de `assets/`; no necesita alias map.

Una herramienta correcta debe:

1. detectar dataset real en `assets/` aunque `res/raw/` exista con 57 stubs;
2. conservar los 384 archivos de assets;
3. no truncar personajes a 13/22/31 durante extracción;
4. preservar PAC bytes;
5. preservar los `.ogg` **byte por byte** aunque su codec real sea MP3 o
   AAC/M4A; desde 00.27 el runtime Vita detecta el contenido y lo decodifica
   directamente, por lo que `prepare_samu_mod.py` no transcodifica;
6. no sobrescribir un save existente con el `save.bin` empaquetado;
7. registrar source APK SHA-256 y, de ser posible, el DEX SHA idéntico a Gen.

## 14. Riesgos de validadores demasiado rígidos

No imponer estas reglas:

- "todo charf tiene PNG": falso;
- "todo charf tiene entries": falso para 20/21;
- "todos los tipos son png/bin/cnv/dac/wav": falso por `u` y `.pn`;
- "todo .ogg es Vorbis": falso para 15 BGM;
- "un roster Gen tiene 13 personajes": falso para este dataset;
- "si DEX no cambia, assets tampoco": falso;
- "si un tipo no dice png, el payload nunca es PNG": falso en `char42 entry 9`.

Toda relajación del runtime debe ser focalizada y respaldada por evidencia del
consumidor original, no por heurísticas generales.

## 15. Clasificación de compatibilidad

Propuesta documental:

- **Tier A:** reemplazos de recursos sobre Gen.
- **Tier B+:** expansión masiva de índices usando el mismo DEX Gen.
- **No Tier C respecto a Gen:** el DEX es idéntico.
- **Audio especial:** 12 BGM MP3 + 3 AAC/M4A con nombres `.ogg`; 00.27 los
  reproduce directamente desde sus bytes originales mediante `SceAudiodec`.
  Los dos Vorbis mantienen el camino libvorbisfile existente.
- **Vita hardware:** roster completo de 92 personajes confirmado en la prueba 00.32; cobertura exhaustiva de todos los personajes/peleas sigue pendiente.

La diferencia entre "data-driven en Android" y "confirmado en Vita" debe
mantenerse explícita.

## 16. Resumen para futuras IAs sin APK

Datos imprescindibles:

```text
APK SHA256:
1771d71de25d664894dfb33b4a296ad6d30f5d897a34eb1ec14f3135496ec41d

DEX SHA256 (idéntico a Gen):
cba71bc13b9d1281aa8180423be9d08db0deb0fc2f5ef6825cc11ba66a17b729

Manifest SHA256 (idéntico a Gen):
0d83c2e0880708eb24c61ec870a87c90a6d08c8b43dae000ce641896b193726e

Assets:
384 total
345 PAC
36 audio con nombre .ogg
loading.png
mk.bin
save.bin

Roster:
char00..91
chardemo00..91
charf0000..0091

Cards:
card000..050

Stages/background:
back00..03
bobj00..04

PAC:
ordinary/non-protected
345/345 valid
6 nested SPR
max image 512x512
92/92 char BIN = 43 records

Nonstandard:
char35/54 + chardemo35/54 => type u URL metadata
char42 entry 9 => type .pn but valid PNG
charf20/21 => empty PAC count=0

Audio:
15/17 BGM non-Vorbis
12/13 are the only baseline Vorbis BGM unchanged
19/19 SE baseline Vorbis unchanged
```

## 17. Regla del proyecto

Este APK **no justifica reescribir el motor**.

Si alguna diferencia impide usar el mod en Vita:

- primero demostrar la diferencia con datos/core/log;
- adaptar VFS/resource/audio/input/render/platform Vita;
- tocar código original/TeaVM generado solo si es totalmente necesario y existe
  evidencia concreta;
- mantener 00.24 como referencia de no-regresión;
- documentar toda decisión, éxito y fallo en su archivo correspondiente;
- hacer commits pequeños.

La meta sigue siendo que **Vita se adapte al contrato del motor**, no que el
motor sea reemplazado por una interpretación nueva para soportar mods.

## 18. Ruta 00.26 histórica — conversión descartada

00.26 demostró que el roster 00..91 podía prepararse y que los 15 BGM
no-Vorbis podían convertirse externamente. Esa solución queda **descartada como
objetivo de compatibilidad** porque exige alterar los audios del mod. Se conserva
solo como registro histórico del intento; no debe usarse para preparar Samu en
la ruta actual.

## 19. Ruta 00.27 — assets originales, audio directo en Vita

La integración actual mantiene el APK como fuente de verdad y no transforma
ningún payload:

```sh
python3 tools/prepare_samu_mod.py \
  DragonBallZuperSamuGamerYT.apk \
  ./install/mods/ZuperSamu
```

El helper:

1. exige el APK SHA-256
   `1771d71de25d664894dfb33b4a296ad6d30f5d897a34eb1ec14f3135496ec41d`;
2. exige el DEX SHA-256
   `cba71bc13b9d1281aa8180423be9d08db0deb0fc2f5ef6825cc11ba66a17b729`;
3. extrae los 384 assets de `assets/` sin cambiar sus bytes;
4. verifica los 92 tripletes `00..91`;
5. verifica por contenido la matriz exacta de 17 BGM:
   **12 MP3 + 3 AAC/M4A + 2 Vorbis**;
6. conserva `payloads_unchanged: true` y registra hashes/codecs observados.

El runtime 00.27 conserva la ruta Vorbis de 00.24 para `bgm_12/13`. Para los
12 MP3 usa el decoder MP3 del sistema Vita mediante `SceAudiodec`. Para
`bgm_09/10/11`, parsea el contenedor ISO-BMFF/M4A, conserva los access units
AAC originales y los entrega al decoder AAC de `SceAudiodec`. El PCM resultante
entra al mismo mezclador de 48 kHz del port.

La auditoría del APK real confirmó AAC-LC 44.1 kHz estéreo. El demux directo
observó:

- `bgm_09`: 228 access units, máximo 1,114 bytes;
- `bgm_10`: 1,578 access units, máximo 1,143 bytes;
- `bgm_11`: 228 access units, máximo 1,114 bytes.

Todos quedan por debajo del límite ES AAC usado por Vita. El parser MP3 validó
los 12 tracks como MPEG Layer III 44.1 kHz estéreo. Estas son pruebas de
formato/build; la reproducción audible, loops y transiciones todavía requieren
la prueba física 00.27.

CI público: Community mod profiles `37544623252` PASS; Vita engine native
smoke `37544588962` PASS con `SceAudiodec_stub`.

Artefacto físico candidato:
`DBTapBattle-Vita-00.27-Samu-DirectAudio-Test.vpk`, SHA-256
`bb13580e6092076d5acca9e9de9cac4b7081e09aeecfcf2761217f3344ebc030`.
Ver [evidencia 00.27](evidence/vita_samu_direct_audio_00.27.json) y
[protocolo físico](TEST_VITA_00_27.md).

## 20. Prueba física 00.28 y corrección 00.29

El usuario probó Samu con `game/` ausente y también con Original instalado. En
ambos casos el runtime reconoce los 92 tripletes y llega al flujo de título, pero
se cierra durante el cambio de BGM. Por tanto el fallo no depende de recursos
prestados desde `game/`.

Los dos logs fallan al crear el decoder de `bgm_00.ogg` con
`sceAudiodecCreateDecoder failed 0x807f0007`. La auditoría directa demuestra que
`bgm_16.ogg` y `bgm_00.ogg` son MP3 válidos y byte-idénticos: no hay evidencia de
un archivo de audio roto. La causa está en el lifetime del adapter Vita: la librería
se inicializa con un único stream MP3 y 00.28 intentaba crear el nuevo decoder antes
de destruir el activo.

00.29 cierra/limpia Voice, stream Vorbis y decoder comprimido bajo el lock de audio
antes de abrir la nueva pista. Los bytes del APK permanecen intactos; no se
transcodifica ningún BGM. Retest físico pendiente.

Desde 00.29 el `save.bin` incluido por este APK ya no se instala como partida del
perfil. Su presencia puede registrarse como evidencia, pero Original/Samu/resto de
mods usan el único `ux0:data/DBTapBattle/save.bin` sembrado por el VPK.

## 21. Confirmación física del roster dinámico — 00.32/00.33

La prueba real posterior a 00.31 confirmó que el bloqueo de Loading desapareció y
que Samu ya muestra **todos sus 92 personajes**. Esto valida en hardware la ruta
dinámica que detecta los tripletes presentes y sincroniza los flags de personaje
del save del perfil, junto con la ampliación verificada de las estructuras del
core necesarias para IDs de dos dígitos 00..99.

00.33 no cambia ese contrato y la prueba de regresión posterior conserva el roster
completo. Esta confirmación no implica que las 92 combinaciones de voces, ataques,
charf y peleas hayan sido probadas una por una; sí cierra el problema específico de
que solo aparecieran los primeros 13 personajes.
