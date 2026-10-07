# APK Español Android14 — referencia técnica completa

Fuente auditada: `DBTB en español para Android 14.apk`

- APK SHA-256: `b38cc2c4ae3f20d1b1c6c1419a7b6b62ab57ea8954468874f6c5f8c40b39a098`
- APK bytes: 71,953,336
- entradas ZIP: 171
- `classes.dex` SHA-256: `d594affc14328decc5a9d898ab8fed52f83c54e2795cf06973454d9f5385a81c`
- `classes.dex`: 701,744 bytes
- perfil PAC: `community14-es-d594affc`

Este documento registra suficiente estructura para implementar/validar soporte sin conservar el APK.

## Manifiesto

El manifiesto es byte-idéntico al Android14 base:

- SHA-256 `e9926f90678aa9297406056601639eb2f1d98683e47ada5b5a70e6a59b539c16`
- package `com.namcobandaigames.apk`
- version `1.4`, versionCode `7`
- minSdk 8, targetSdk 29
- `debuggable=true`, `hardwareAccelerated=true`
- launcher `.Primary`
- BluetoothSearch original conservado
- permisos: Internet, Vibrate, Write External Storage, Bluetooth, Bluetooth Admin
- sin billing permission, Smap activity, BillingService ni BillingReceiver.

Firma: certificado Android genérico, fingerprint SHA-256
`a40da80a59d170caa950cf15c18c454d47a39b26989d8b640ecd745ba71bf5dc`.

## DEX

Dimensiones idénticas a Android14:

- 781 strings
- 169 types
- 252 protos
- 1,219 fields
- 914 method IDs
- 39 classes
- 596 métodos con `code_item`

Estructura de clases:

- 35 clases `ext.*`
- `Lcom/namcobandaigames/apk/Primary;`
- `BluetoothSearch`
- `BluetoothSearch$BluetoothClientThread`
- `BluetoothSearch$BluetoothServerThread`.

Comparación por firma e instrucciones con Android14:

- 596/596 métodos con código tienen firma común.
- 585 son byte-idénticos en sus instrucciones.
- 11 cambian.

Los 11 cambios están limitados a:

- `BluetoothSearch$BluetoothServerThread.<init>`
- `ext.E.a(Context,String):boolean`
- `ext.G.b(GL10,int,int):boolean`
- `ext.c.a(GL10)`
- `ext.c.a(GL10,byte[])`
- dos métodos de `ext.m`
- `ext.o.<clinit>`
- `ext.w.a(byte[])`
- dos métodos de `ext.y`.

Esto encaja con cambios de perfil/loader/render/text y no con una reescritura general comparable a Invasion.

## Dataset

`assets/` contiene 144 archivos:

- 106 PAC protegidos
- 17 BGM
- 19 SE
- `loading.png`
- `mk.bin`

No incluye `save.bin`.

Tras canonicalizar aliases:

- `back00..03`
- `bobj01..04` (no `bobj00`)
- `card000..050`
- `char00..12`
- `chardemo00..12`
- `charf0000..0012`
- common/select/effect/demo/card_preview/gamedata/text00.

No incluye `font00.pac`.

La ausencia de `bobj00`/`font00` es válida en este APK autónomo. En 00.28 el perfil no usa fallback desde `game/`; si el core original TeaVM solicita uno de esos recursos, la diferencia debe adaptarse explícitamente desde evidencia del APK/DEX.

## Alias exactos

- `4D7F` → `common`
- `B4EB` → `select0`
- `B248` → `effect`
- `AC3B` → `demo_00`
- `8827` → `demo_08`
- `4919` → `card_preview`
- `EC5A` → `gamedata`
- `A602` → `text00`
- `0294XX` → `backXX`
- `D794XX` → `bobjXX`
- `F298XX` → `charXX`
- `AE52XX` → `chardemoXX`
- `EB21XXXX` → `charfXXXX`
- `6FA6XXX` → `cardXXX`.

## Perfil binario

```text
count XOR       0xE6AA
offset XOR      0x31874C24
size XOR        0x790E6BAF
image width XOR 0x29CD
image height XOR 0x42AC
table count XOR 0x2EA3
table pos XOR   0x07DB0921
table width XOR 0x941F
table height XOR 0x126F
WAV size XOR    0xE6AA
```

Claves de tipo observadas después de `raw_be32 ^ entry_index`:

- BIN `0x00FC517E`
- CNV `0xEDB419C8`
- DAC `0xD49FADE7`
- RGBA `0x5EE0F896`
- SPR `0x03C296FD`
- WAV `0x5BAD42A6`

No se observó top-level ACT en este corpus; no inventar una clave.

## Corpus PAC

106/106 PAC exteriores validan únicamente con este perfil. Contando seis SPR anidados:

- RGBA 390 (361 top-level + 29 nested)
- WAV 198
- CNV 106
- DAC 106
- BIN 74 (68 top-level + 6 nested)
- SPR 6
- metadata desconocida 5.

Máxima imagen decodificada: 512×512.

### Tablas

- `gamedata.pac`: 271 registros; payload normalizado 87,435 bytes; 39,811 celdas.
- `text00.pac`: 1 registro; 25,012 bytes; 315 celdas.
- cada `char00..12`: BIN con 43 registros.
- cada `card000..050`: BIN con 1 registro.
- cada `back00..03`: BIN con 3 registros.

`text00` contiene cadenas UTF-8 españolas. El texto no debe seleccionarse por codec del contenedor; el runtime debe detectar charset a nivel del contenido que consume.

## Outlier de tabla: card034

`card034.pac` es un caso válido que no debe confundirse con corrupción: su BIN
convertido contiene **1 registro con dimensión 0×0**, por tanto 0 celdas. El
payload de tabla normalizado ocupa 10 bytes. Invasion conserva exactamente este
mismo caso. Un validador genérico no debe imponer `width > 0 && height > 0`
para todas las tablas convertidas; debe validar bounds y contrato real del
consumidor.

## Audio exterior

Los 36 archivos son byte-idénticos al Original/Gen/Android14:

- 36 Vorbis reales
- 44.1 kHz
- 17 BGM stereo
- 19 SE mono.

No hay problema de extensión falsa en este APK.

## Diferencias reales contra Android14

Aunque estructura, aliases lógicos y audio exterior son equivalentes, el contenido no es solo una traducción de `text00`.

Comparación top-level de tipos plenamente normalizados:

- imágenes RGBA: 304 iguales / 57 diferentes.
- voces PCM: 39 iguales / 159 diferentes.
- tablas convertidas: 5 iguales / 65 diferentes.

Los 106 PAC conservan exactamente la misma secuencia top-level de tipos que Android14. Por tanto, las diferencias están dentro de payloads, no en un rediseño de la estructura del contenedor.

## Relación con Invasion

En los 106 PAC compartidos, Invasion está mucho más cerca de este APK que del Android14 base:

- 303/361 imágenes top-level iguales
- 195/198 voces PCM iguales
- 52/70 tablas normalizadas iguales.

Esto es evidencia de linaje técnico de contenido, no prueba de autoría.

## Contrato Vita

- usar codec por PAC, no global.
- canonicalizar nombres sin modificar bytes extraídos.
- mantener el perfil autónomo: no copiar `bobj00`/`font00` desde `game/`; investigar cualquier solicitud del core original como una diferencia de compatibilidad.
- no ejecutar `classes.dex` del mod.
- no cambiar el motor original para acomodar esta traducción.
- cualquier diferencia de comportamiento no explicada por datos requiere evidencia DEX/método concreta antes de tocar un adapter Vita.
