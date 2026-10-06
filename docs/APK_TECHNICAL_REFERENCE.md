# Dragon Ball Tap Battle — referencia técnica de los APK auditados

Fecha de auditoría profunda: **2026-10-06**.

Este documento existe para que el port de PS Vita pueda seguir desarrollándose **sin necesitar los APK originales/modificados a mano**. No contiene payloads comerciales, clases decompiladas ni assets; registra estructura, hashes, tamaños, contratos binarios, inventarios y diferencias observadas directamente en los cinco APK suministrados al proyecto.

La evidencia máquina-a-máquina correspondiente está en `docs/evidence/apk_deep_structure_2026-10-06.json`.

## Regla de interpretación

Los niveles de evidencia deben mantenerse separados:

- **OBSERVADO EN APK:** bytes, hashes, estructura ZIP, manifiesto, DEX, PAC, audio y librerías comprobados directamente.
- **NORMALIZADO/DECODIFICADO:** contenido comparado después de quitar únicamente protecciones/metadatos cuyo contrato está confirmado.
- **INFERENCIA:** relación probable entre mods o intención de un cambio; nunca tratar como hecho si no hay evidencia directa.
- **HARDWARE CONFIRMED:** solo lo probado en PS Vita real. La referencia estable sigue siendo 00.24 hasta que 00.25 sea probado físicamente.

No se debe reconstruir ni modificar lógica original del motor a partir de estos documentos. El motor original AOT sigue siendo la autoridad; Vita debe adaptar sus servicios al contrato del motor.

---

## 1. Identidad de las cinco fuentes

| ID documental | Archivo suministrado | SHA-256 APK | Bytes | Entradas ZIP |
|---|---|---|---:|---:|
| `original` | `DBTapBattle.apk` | `b84f98a3ed70957354f358b7930bd8fb651cc89b74e16f8774ebd989fbf0899b` | 16,050,451 | 77 |
| `gen` | `gen.apk` | `d52cbd7ef248d995ad17ba6ec8ec6fa08590a344ac2a9786e5ac839bf7715f28` | 78,935,671 | 224 |
| `android14` | `tap battle android 14.apk` | `a210795bf7ded8636a91bea96df051557229149feb310cf07baf16b0731e79c4` | 72,391,449 | 171 |
| `spanish14` | `DBTB en español para Android 14.apk` | `b38cc2c4ae3f20d1b1c6c1419a7b6b62ab57ea8954468874f6c5f8c40b39a098` | 71,953,336 | 171 |
| `invasion_b3` | `TAP BATTLE INVASION BETA 3.apk` | `caaf294ddb9bf833868d7b541fc310603827bed44072230f60e0552cbb2dc94d` | 124,787,316 | 204 |

### DEX

| Perfil | `classes.dex` SHA-256 | Bytes | strings | types | protos | fields | methods | classes |
|---|---|---:|---:|---:|---:|---:|---:|---:|
| original | `0e2de1f3d712454aa30175016b0e2987c4d7e8e36bddab48ef4a2037023ecb72` | 704,916 | 3,503 | 311 | 471 | 1,269 | 1,368 | 91 |
| gen | `cba71bc13b9d1281aa8180423be9d08db0deb0fc2f5ef6825cc11ba66a17b729` | 700,624 | 3,513 | 310 | 473 | 1,264 | 1,369 | 90 |
| Android14 | `f4e52c47fac7f1f6288c4bf7e4d31bc819ebb6ec2d2a6afac2c0275602995184` | 701,752 | 781 | 169 | 252 | 1,219 | 914 | 39 |
| Español | `d594affc14328decc5a9d898ab8fed52f83c54e2795cf06973454d9f5385a81c` | 701,744 | 781 | 169 | 252 | 1,219 | 914 | 39 |
| Invasion B3 | `05aa0c5ec839161e59b93eccd8657925380c56b46f1a4a452c662b1f115212d1` | 701,740 | 781 | 169 | 252 | 1,219 | 914 | 39 |

Los tres APK protegidos tienen exactamente las mismas dimensiones de tablas DEX. Eso **no** implica mismo comportamiento: Invasion modifica una parte grande de los métodos con código.

---

## 2. AndroidManifest y empaquetado Android

### Original

- package: `com.namcobandaigames.dragonballtap.apk`
- versionName/versionCode: `1.4` / `7`
- minSdkVersion: `9`
- targetSdkVersion: no declarado en el manifiesto suministrado.
- `hardwareAccelerated=true`.
- launcher: `.dragonballtap`.
- actividades adicionales: `.BluetoothSearch`, `.smap`.
- servicio: `jp.co.bandainamcogames.Smap.Market.BillingService`.
- receiver: `jp.co.bandainamcogames.Smap.Market.BillingReceiver`.
- permisos: Internet, Vibrate, Write External Storage, Bluetooth, Bluetooth Admin y Google Play Billing.
- no `lib/*.so`.

Firma/certificado:

- sujeto: `NAMCO BANDAI Games Inc.`
- serial: `4CC13C7A`
- fingerprint SHA-256 del certificado: `336ca2245718a9ca1672bf0bf2d324b29a836d899848ac0e8c08ba79097c03b3`

### Gen

Conserva package, versión, launcher, actividades, Smap/billing y permisos del original, pero:

- `android:debuggable=true`.
- firma con el certificado genérico de Android/debug, no el certificado NAMCO BANDAI.
- el DEX pierde `Downloader$DownloadTask` y modifica la estructura de algunas clases de plataforma (`Downloader`, `SoundEffect`, `Utility`).
- mantiene 57 nombres en `res/raw/`, pero **todos son archivos de cero bytes**.
- los datos reales se trasladan a `assets/`.

### Android14 y Español

Ambos manifiestos son byte-idénticos (`AndroidManifest.xml` SHA-256 `e9926f90678aa9297406056601639eb2f1d98683e47ada5b5a70e6a59b539c16`) y declaran:

- package: `com.namcobandaigames.apk`
- versionName/versionCode: `1.4` / `7`
- minSdkVersion: `8`
- targetSdkVersion: `29`
- `debuggable=true`
- launcher: `.Primary`
- actividad Bluetooth: `com.namcobandaigames.dragonballtap.apk.BluetoothSearch`
- sin actividad Smap, BillingService ni BillingReceiver.
- permisos conservados: Internet, Vibrate, Write External Storage, Bluetooth y Bluetooth Admin.
- se elimina `com.android.vending.BILLING`.

### Invasion Beta 3

Conserva casi el mismo manifiesto protegido, pero cambia el namespace principal:

- package: `rinne.private.apk`
- launcher: `.Primary`, que corresponde a `Lrinne/private/apk/Primary;`.
- minSdk 8 / targetSdk 29.
- el resto de permisos/actividad Bluetooth permanece en el mismo modelo del Android14 protegido.

Su manifiesto ya no es byte-idéntico al de Android14/Español.

### Firma de Gen + los tres APK protegidos

Los cuatro usan el mismo certificado Android genérico:

- serial: `936EACBE07F201DF`
- fingerprint SHA-256: `a40da80a59d170caa950cf15c18c454d47a39b26989d8b640ecd745ba71bf5dc`
- sujeto: Android / `android@android.com`.

Esto es evidencia de la clave usada para firmar, **no** de autoría.

---

## 3. Layout de datos de juego

| Perfil | Ubicación real | Archivos de datos | PAC | audio con extensión `.ogg` | otros |
|---|---|---:|---:|---:|---|
| original | `res/raw/` | 57 | 19 | 36 | `loading.png`, `mk.bin` |
| gen | `assets/` | 147 | 108 | 36 | `loading.png`, `mk.bin`, `save.bin` |
| Android14 | `assets/` | 144 | 106 | 36 | `loading.png`, `mk.bin` |
| Español | `assets/` | 144 | 106 | 36 | `loading.png`, `mk.bin` |
| Invasion B3 | `assets/` | 177 | 139 | 36 | `loading.png`, `mk.bin` |

### Archivos compartidos no-PAC

`mk.bin` es idéntico en los cinco perfiles:

- 392 bytes
- SHA-256 `2caef8c445b71896f34d560e90be6e04d1911d6bf8010d73c84dd9288b30251f`

`loading.png`:

- todos son 128×128.
- original: 4,233 bytes, SHA-256 `79b374ee879104a5ae040c8f3308fcb225c89a8fd2e8696306ed2258cc1c95bd`.
- Gen/Android14/Español/Invasion: 3,394 bytes, SHA-256 `92565342ed5d33ce05400d1f4cd9ada2159bd83f2f87e55a12f897c8a70c817b`.

Solo Gen incluye `save.bin` dentro del dataset:

- 12,906 bytes
- SHA-256 `8b65f591ca2ba4724af55bfab4adf9e8a4260c7cd517f6bd93aa4c69a1239714`.

No se debe asumir que ese save es apropiado para reemplazar progreso existente.

---

## 4. Inventario lógico de contenido

### Original suministrado

Contiene solamente el conjunto base:

- `back00..03`
- `bobj00..04`
- `card000`
- `card_preview`
- `common`
- `demo_00`, `demo_08`
- `effect`
- `font00`
- `gamedata`
- `select0`
- `text00`
- 17 BGM (`bgm_00..16`)
- 19 SE (`se_00..18`)

**No contiene `charXX`, `chardemoXX`, `charfXXXX` ni `card001..050`.** El APK original suministrado no es por sí solo una instalación offline completa para batalla.

### Gen

Añade sobre el conjunto lógico completo:

- 13 `char00..12`
- 13 `chardemo00..12`
- 13 `charf0000..0012`
- `card000..050` (51 cartas)
- conserva `back00..03`
- conserva `bobj00..04`
- conserva `font00`
- incluye `save.bin`.

Total: 108 PAC.

### Android14 y Español

Después de resolver aliases, ambos exponen exactamente 106 PAC lógicos:

- 13 tripletes de personajes (`00..12`)
- 51 cartas (`000..050`)
- `back00..03`
- `bobj01..04`
- recursos comunes.

Ausencias deliberadas en el APK:

- `bobj00.pac`
- `font00.pac`

Por eso requieren fallback desde `game/` cuando se usan como overlay en Vita.

### Invasion Beta 3

Expande el mismo esquema a:

- **22** `char00..21`
- **22** `chardemo00..21`
- **22** `charf0000..0021`
- 51 cartas `card000..050`
- 7 backgrounds `back00..06`
- 7 objetos `bobj01..07`
- mismos recursos comunes de la familia protegida.

Total: 139 PAC, es decir **33 PAC más** que Android14/Español:

- 9 `char13..21`
- 9 `chardemo13..21`
- 9 `charf0013..0021`
- 3 `back04..06`
- 3 `bobj05..07`.

También omite `bobj00.pac` y `font00.pac`.

---

## 5. Formato PAC ordinario

El original y Gen usan PAC ordinario:

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

Los offsets son relativos a `data_base`.

### Corpus original suministrado

19 PAC exteriores; contando SPR anidados:

- PNG: 80
- CNV: 19
- DAC: 19
- BIN: 12
- GDT: 7
- DB: 7
- SPR: 6
- ACT: 4
- DAT: 4
- PLT: 4
- BMP: 3

No hay WAV de personajes porque no hay PAC `charXX` en este APK.

Máxima imagen observada: 512×512.

### Corpus Gen

108 PAC exteriores; contando SPR anidados:

- PNG: 405
- WAV: 198
- CNV: 108
- DAC: 108
- BIN: 75 (69 top-level + 6 dentro de SPR)
- SPR: 6
- PLT: 5
- ACT: 4

Los 198 WAV son RIFF PCM ordinario. Máxima imagen observada: 512×512.

Todos los 13 `charXX.pac` tienen un BIN convertido de **43 registros**. Las 51 cartas contienen un BIN convertido de un registro. Los cuatro backgrounds tienen BIN convertido de tres registros.

---

## 6. Familia PAC protegida Android14 / Español / Invasion

Los tres usan una tabla exterior de 16 bytes por entrada con campos ofuscados por XOR. El índice `i` es el índice de entrada dentro del contenedor actual. Un `spr` anidado reinicia `i=0`.

Fórmula general:

```text
count       = raw_u16_le ^ COUNT_XOR
offset[i]   = raw_u32_le ^ OFFSET_XOR ^ i
size[i]     = raw_u32_le ^ SIZE_XOR ^ i
encodedType = raw_u32_be ^ i
```

`encodedType` se compara con las claves de tipo específicas del perfil.

### Constantes

| Campo | Android14 | Español | Invasion B3 |
|---|---:|---:|---:|
| count XOR | `0xA732` | `0xE6AA` | `0x842F` |
| offset XOR | `0x3B681C6B` | `0x31874C24` | `0x71573ADB` |
| size XOR | `0x02D6D26E` | `0x790E6BAF` | `0x33AC6051` |
| image width XOR | `0xF34D` | `0x29CD` | `0xA42C` |
| image height XOR | `0x93F9` | `0x42AC` | `0xED15` |
| table count XOR | `0x8722` | `0x2EA3` | `0x68A3` |
| table position XOR | `0x8F7FC2CA` | `0x07DB0921` | `0x122E64CB` |
| table width XOR | `0x5ADD` | `0x941F` | `0xB1D9` |
| table height XOR | `0xB9E8` | `0x126F` | `0x7C00` |
| WAV decoded-size XOR | `0xA732` | `0xE6AA` | `0x842F` |

Las claves semánticas exactas están registradas en `src/community_profiles.hpp` y en el evidence JSON. No deben inventarse claves para tipos no observados.

### Imágenes RGBA protegidas

Payload:

```text
u16_be encoded_width
u16_be encoded_height
raw-DEFLATE RGBA bytes
```

```text
width  = encoded_width  ^ IMAGE_WIDTH_XOR  ^ entry_index
height = encoded_height ^ IMAGE_HEIGHT_XOR ^ entry_index
```

El stream DEFLATE es raw (`windowBits=-15`) y debe producir exactamente `width * height * 4` bytes. La imagen está premultiplicada.

Máximos observados:

- Android14: 512×512.
- Español: 512×512.
- Invasion: **736×500**, 1,472,000 bytes RGBA, dentro de `char15.pac`.

### Tablas GameData convertidas

Payload ordinario lógico:

```text
u16_le count
count × {
    u32_le position
    u16_le width
    u16_le height
}
record payload bytes...
```

En los perfiles protegidos esos cuatro campos se XOR con las constantes de tabla y con el índice del registro de tabla.

Observaciones:

- `gamedata.pac`: 271 registros en Gen, Android14, Español e Invasion.
- `text00.pac`: 1 registro en los cuatro.
- cada `charXX.pac`: 43 registros BIN.
- cada carta: 1 registro BIN.
- cada `backXX.pac`: 3 registros BIN.
- Invasion mantiene 43 registros también en `char13..21`, lo que demuestra continuidad estructural del formato.

### WAV protegido

Wrapper:

```text
u32_le encoded_decoded_pcm_size
u8 compression_flag
payload...
```

```text
decoded_pcm_size = encoded ^ WAV_SIZE_XOR ^ outer_entry_index
```

Con flag 0, después del byte 4 hay PCM envuelto. Los streams comprimidos observados usan frames PlayStation ADPCM de 16 bytes, predictor/shift en byte 0 y control en byte 1; el decoder debe producir exactamente la longitud PCM declarada.

No confundir estos WAV internos con los ficheros de música `bgm_XX.ogg`.

---

## 7. Alias protegidos → nombres canónicos

| Recurso lógico | Android14 | Español | Invasion |
|---|---|---|---|
| common | `2752` | `4D7F` | `9036` |
| select0 | `1BC2` | `B4EB` | `7E8F` |
| effect | `9B28` | `B248` | `1E1C` |
| demo_00 | `59F2` | `AC3B` | `97E6` |
| demo_08 | `3C90` | `8827` | `6E24` |
| font00 | `D0BD` (ausente) | no observado/bundled | `71DC` (ausente) |
| card_preview | `5D73` | `4919` | `0708` |
| gamedata | `D67E` | `EC5A` | `90EA` |
| text00 | `82B7` | `A602` | `D37C` |
| backXX | `0B49XX` | `0294XX` | `F813XX` |
| bobjXX | `BDC7XX` | `D794XX` | `17A5XX` |
| charXX | `E03BXX` | `F298XX` | `0953XX` |
| chardemoXX | `8AC1XX` | `AE52XX` | `364EXX` |
| charfXXXX | `FAFDXXXX` | `EB21XXXX` | `91F9XXXX` |
| cardXXX | `47DDXXX` | `6FA6XXX` | `1A4BXXX` |

El extractor debe **renombrar paths, no reescribir los bytes del PAC**.

---

## 8. Composición PAC protegida

Contando contenedores exteriores y entradas de SPR anidados:

| Tipo | Android14 | Español | Invasion |
|---|---:|---:|---:|
| RGBA | 390 | 390 | 763 |
| WAV | 198 | 198 | 344 |
| CNV | 106 | 106 | 139 |
| DAC | 106 | 106 | 139 |
| BIN | 74 | 74 | 89 |
| SPR | 6 | 6 | 9 |
| unknown/metadata | 5 | 5 | 5 |

Top-level únicamente:

- Android14/Español: 361 RGBA, 198 WAV, 68 BIN.
- Invasion: 731 RGBA, 344 WAV, 80 BIN.

Todos los 106/106 PAC Android14, 106/106 Español y 139/139 Invasion pasan validación de directorio con su **único** perfil correspondiente; no hubo offsets fuera de archivo.

---

## 9. Audio exterior: descubrimiento crítico de Invasion

### Baseline Original / Gen / Android14 / Español

Los 36 archivos (`bgm_00..16`, `se_00..18`) son **byte-idénticos entre los cuatro perfiles**.

- 36/36: codec Vorbis real.
- sample rate: 44,100 Hz.
- BGM: 17 stereo.
- SE: 19 mono.

### Invasion

29/36 permanecen byte-idénticos al baseline. Cambian exactamente:

`bgm_03`, `bgm_04`, `bgm_05`, `bgm_06`, `bgm_07`, `bgm_14`, `bgm_15`.

**Aunque conservan extensión `.ogg`, esos siete archivos NO son Vorbis:**

| Archivo | codec real | contenedor/magic observado | Hz | ch | duración | bitrate | bytes | PCM16 stereo aprox. |
|---|---|---|---:|---:|---:|---:|---:|---:|
| bgm_03.ogg | MP3 | frame MPEG (`FF FB`) | 44100 | 2 | 111.830 s | 256 kb/s | 3,578,566 | 18.81 MiB |
| bgm_04.ogg | AAC | ISO BMFF/M4A (`ftypM4A`) | 44100 | 2 | 13.142 s | ~125 kb/s | 208,708 | 2.21 MiB |
| bgm_05.ogg | AAC | ISO BMFF/M4A (`ftypM4A`) | 44100 | 2 | 195.419 s | ~125 kb/s | 3,087,225 | 32.87 MiB |
| bgm_06.ogg | MP3 | frame MPEG (`FF FB`) | 48000 | 2 | 99.000 s | 256 kb/s | 3,168,000 | 18.13 MiB |
| bgm_07.ogg | MP3 | ID3 + MPEG | 48000 | 2 | 127.104 s | 160 kb/s | 2,970,086 | 23.27 MiB |
| bgm_14.ogg | MP3 | frame MPEG (`FF FB`) | 44100 | 2 | 138.188 s | 256 kb/s | 4,422,008 | 23.25 MiB |
| bgm_15.ogg | MP3 | ID3 + MPEG | 44100 | 2 | 85.368 s | 160 kb/s | 1,707,930 | 14.36 MiB |

Consecuencia para Vita: el servicio actual usa `libvorbisfile` / `ov_fopen()`. Ese API **no puede abrir MP3 ni AAC/M4A aunque el nombre termine en `.ogg`**. Por tanto, el soporte de Invasion no está completo solo con streaming Vorbis. Antes de afirmar compatibilidad de BGM se necesita una adaptación Vita con evidencia, por ejemplo:

1. detectar codec por magic/contenido, no extensión;
2. implementar decoder Vita/adaptador apropiado para MP3/AAC, o convertir en la herramienta de importación a un formato ya soportado manteniendo manifest/hashes de origen;
3. no cambiar la lógica del motor: `SoundEffect/MediaPlayer` debe seguir pidiendo el mismo nombre lógico; la capa Vita resuelve el formato real.

Este punto invalida cualquier documentación anterior que describa los siete BGM cambiados de Invasion como “Vorbis largo”.

---

## 10. Comparación Original ↔ Gen

De los 57 nombres de datos del original, Gen conserva los 57 nombres en `assets/`:

- 40 son byte-idénticos.
- 17 cambian.
- Gen añade 90 archivos lógicos adicionales.

Byte-idénticos destacados:

- los 36 audios exteriores;
- `mk.bin`;
- `demo_08.pac`, `effect.pac`, `font00.pac`.

Cambian:

- `back00..03`
- `bobj00..04`
- `card000`
- `card_preview`
- `common`
- `demo_00`
- `gamedata`
- `loading.png`
- `select0`
- `text00`.

Gen añade exactamente los 39 PAC de personaje, 50 cartas adicionales y `save.bin`.

### DEX Original ↔ Gen

Ambos conservan la arquitectura de clases original y nombres legibles. Diferencias estructurales observadas:

- Original: 91 clases; Gen: 90.
- falta `Downloader$DownloadTask` en Gen.
- `Downloader`: 10 métodos/7 campos original → 9 métodos/1 campo Gen.
- `SoundEffect`: 20 métodos/18 campos → 21/19.
- `Utility`: 21 métodos/3 campos → 22/6.
- Gen está marcado `debuggable=true`.

No usar un hash bruto de instrucciones para declarar que cientos de métodos cambiaron semánticamente: el reordenamiento de pools DEX puede cambiar índices embebidos en opcodes. Para Original↔Gen hace falta decompilación/IR semántico antes de atribuir una diferencia de comportamiento.

---

## 11. Android14 ↔ Español

Son extremadamente cercanos en estructura:

- mismo package/manifiesto.
- mismas dimensiones DEX: 39 clases, 914 methods IDs, 596 métodos con código.
- mismo conjunto de 106 PAC lógicos.
- mismos 36 audios exteriores byte a byte.
- mismas siete `libabc.so` por ABI, byte a byte.
- los 106 PAC tienen la misma secuencia de tipos top-level después de decodificar su perfil.

### DEX

De 596 métodos con código y misma firma:

- **585** tienen instrucciones byte-idénticas.
- **11** difieren.

Métodos con instrucciones diferentes:

- `BluetoothSearch$BluetoothServerThread.<init>`
- `ext.E.a(Context,String):boolean`
- `ext.G.b(GL10,int,int):boolean`
- `ext.c.a(GL10):void`
- `ext.c.a(GL10,byte[]):boolean`
- dos métodos de `ext.m`
- `ext.o.<clinit>`
- `ext.w.a(byte[]):int`
- dos métodos de `ext.y`.

La mayor parte son loader/platform/profile constants; no hay evidencia para afirmar cambios masivos de lógica de juego.

### Contenido decodificado top-level

Comparando únicamente tipos completamente normalizados/decodificados:

- RGBA: 304 iguales / 57 diferentes (361 total).
- PCM de voces: 39 iguales / 159 diferentes (198 total).
- tablas convertidas: 5 iguales / 65 diferentes (70 total).

Esto demuestra que Español **no es solo text00 traducido**: modifica voces, datos y parte de los gráficos. No atribuir el significado de cada diferencia sin análisis específico.

`text00` contiene cadenas UTF-8 en español y conserva 1 registro convertido / 315 celdas de tabla.

---

## 12. Android14 ↔ Invasion

En los 106 PAC canónicos compartidos:

- 105 conservan la misma secuencia de tipos top-level.
- `demo_00.pac` de Invasion añade una RGBA extra antes de CNV/DAC/metadata.
- Invasion añade 33 PAC completamente nuevos.

Contenido top-level completamente decodificado:

- imágenes: 250 iguales / 111 diferentes frente a Android14.
- voces PCM: 39 iguales / 159 diferentes.
- tablas convertidas: 5 iguales / 65 diferentes.

### DEX

Android14 e Invasion tienen 596 métodos con código cada uno.

- 595 firmas de método con código son comunes.
- solo **223** conservan instrucciones idénticas.
- **372** cambian instrucciones.
- Android14 tiene `Lcom/namcobandaigames/apk/Primary;-><init>()V`.
- Invasion reemplaza esa firma por `Lrinne/private/apk/Primary;-><init>()V`.

Los cambios se distribuyen ampliamente por clases `ext.*`, especialmente `ext.C` (136 métodos diferentes) y `ext.A` (91). Esto es evidencia fuerte de un **mod de código (Tier C)** además de un mod de datos.

No se debe asumir que cargar 22 personajes automáticamente reproduce todos los cambios mecánicos de Invasion. El Vita port no ejecuta su `classes.dex`.

---

## 13. Español ↔ Invasion: evidencia de linaje técnico

Esta comparación es especialmente útil.

En los 106 PAC compartidos, después de normalización de tipos plenamente comprendidos:

- imágenes top-level: **303 iguales / 58 diferentes**.
- voces PCM: **195 iguales / 3 diferentes**; las tres diferencias están dentro de `char11.pac`.
- tablas convertidas: **52 iguales / 18 diferentes**.

Las tablas diferentes están concentradas en:

- `card048..050`
- `char00..12`
- `gamedata`
- `text00`.

Invasion además conserva el texto español de la misma familia, añade personajes 13..21 y escenarios/objetos adicionales.

Esto es **evidencia técnica fuerte de que Invasion parte de una base de contenido muy cercana al APK Español**, pero no prueba autoría, fecha de fork ni procedencia social del mod.

---

## 13.1. Diferencia semántica por PAC protegido

Comparar el SHA-256 exterior de dos PAC protegidos no sirve para decidir si su
contenido cambió: cada perfil usa aliases/XOR distintos. Para esta auditoría se
calculó además un digest semántico por PAC usando **solo contratos confirmados**:
RGBA decodificado/premultiplicado, PCM de WAV interno decodificado y tablas
GameData con su cabecera normalizada. CNV/DAC crudo, metadata desconocida y
mecánicas DEX no se reinterpretan.

### Android14 → Español

106 PAC canónicos compartidos:

- 7 conservan el mismo digest semántico: `back00..03`, `card000`,
  `card_preview`, `demo_08`.
- 99 cambian al menos un componente normalizado.

Esto confirma que Español no es una simple sustitución de `text00`.

### Español → Invasion Beta 3

106 PAC canónicos compartidos:

- **81 conservan el mismo digest semántico**.
- **25 cambian**:

```text
card048.pac
card049.pac
card050.pac
char00.pac
char01.pac
char02.pac
char03.pac
char04.pac
char05.pac
char06.pac
char07.pac
char08.pac
char09.pac
char10.pac
char11.pac
char12.pac
chardemo08.pac
charf0008.pac
charf0011.pac
demo_00.pac
demo_08.pac
effect.pac
gamedata.pac
select0.pac
text00.pac
```

Los otros 81 incluyen `back00..03`, `bobj01..04`, `card000..047`,
la mayoría de `chardemo`/ `charf` del rango original y `common.pac`.
A esto se suman los 33 PAC nuevos exclusivos de Invasion
(`char13..21`, `chardemo13..21`, `charf0013..0021`,
`back04..06`, `bobj05..07`).

Este conjunto de 25 es una guía de investigación, no una lista automática de
código que deba portarse: un PAC puede cambiar por arte/voz/datos sin requerir
cambio de motor, y una mecánica puede cambiar en DEX aunque su PAC sea idéntico.

### Android14 → Invasion

De los 106 PAC compartidos, solo 6 conservan el digest semántico completo
(`back00..03`, `card000`, `card_preview`). La comparación mucho más cercana
Español→Invasion refuerza la relación técnica entre esos dos datasets.

## 14. Librerías nativas de la familia protegida

Android14, Español e Invasion contienen `libabc.so` en siete ABIs y **cada par correspondiente es byte-idéntico entre los tres APK**:

| ABI | bytes | SHA-256 |
|---|---:|---|
| arm64-v8a | 9,696 | `7e98a974c49f24b38e85cff61a4c18bc3f9bb794b173e3e09252105446815972` |
| armeabi-v7a | 13,544 | `9b4f5656dc8409e9c6b90e5805b1a2206b201a044ecc44ac72e8f66ef8039cf3` |
| armeabi | 17,632 | `da48638a563d4841541cfedf8aa7e620f7f62534cb5513f6565822ea593d7fe5` |
| mips | 71,388 | `f144130143a12b2354773db688b9ba18dc9d01c37e867ba7075a1102445383d4` |
| mips64 | 14,704 | `a2d2e0a64502aea6b23ab2b089f03482736689c90fb0b20f2012d32581b3b7c6` |
| x86 | 9,368 | `fc03d01b3fe37fdf56e7a6372a183e870a41c2f50a8a823f285244e7547066a0` |
| x86_64 | 9,936 | `566f0f4f78cd889ffe1767a27b99039b234fd77e6be3cb3fdcac5c6776f06633` |

Conclusión válida: comparten el mismo helper nativo. Conclusión **no** válida: “sus DEX o reglas de juego son iguales”.

---

## 15. Implicaciones directas para el port Vita

1. **Original/Gen** son PAC ordinarios. No aplicarles perfil protegido.
2. **Android14/Español/Invasion** requieren detección por PAC, no una variable global de “mod activo”, porque el VFS puede mezclar override protegido + fallback original ordinario.
3. Un perfil solo se acepta cuando su directorio completo está dentro de bounds y el match es único.
4. `bobj00` y `font00` faltantes deben provenir del base compatible; no fabricar archivos vacíos.
5. Invasion necesita aceptar 22 tripletes contiguos, pero eso no autoriza a reemplazar arrays/lógica del motor sin evidencia.
6. Las imágenes Invasion llegan hasta 736×500; no introducir límites de 512×512.
7. El audio de Invasion exige detección real de codec; extensión `.ogg` no es suficiente.
8. Las diferencias de DEX de Invasion obligan a investigar una mecánica concreta antes de portar comportamiento. No copiar masivamente código del mod ni reescribir el motor.
9. Toda adaptación debe ocurrir en VFS/codec/audio/render/input/platform adapters cuando sea posible.
10. Cualquier cambio sobre una ruta confirmada en 00.24 necesita evidencia del fallo, prueba de regresión y documentación.

---

## 16. Dónde está la información exacta

- Perfil/constantes runtime: `src/community_profiles.hpp`
- Normalización: `src/engine_resources.cpp`
- PAC reader: `src/pac.cpp`
- GameData convertido: `src/game_data.cpp`
- VFS/fallback: `src/vfs.cpp`
- gate de tripletes: `src/installed_data.cpp`
- extractor Python: `tools/community14.py`, `tools/extract_apk_data.py`
- extractor Windows: `tools/windows/Extraer_APK_para_Vita.ps1`
- referencia Android14: `docs/ANDROID14_APK.md`
- Gen: `docs/ORIGINAL_PLUS_CHARACTERS_APK.md`
- Español: `docs/SPANISH_ANDROID14_APK.md`
- Invasion: `docs/INVASION_BETA3_APK.md`
- evidencia exacta sin payloads: `docs/evidence/apk_deep_structure_2026-10-06.json`

## 17. Regla para futuros APK/mods

Nunca declarar “compatible” un APK nuevo por nombre, tamaño o apariencia. Registrar como mínimo:

- SHA-256 APK y DEX;
- manifest/package/minSdk/target/launcher;
- certificado;
- inventario ZIP/data;
- esquema de nombres/aliases;
- perfil PAC y match único por archivo;
- tipos/entry counts/bounds;
- máximos de imagen;
- tablas GameData;
- audio detectado por magic/codec, no extensión;
- tripletes de personajes completos y contiguos;
- cambios DEX frente a un perfil conocido;
- recursos ausentes que requieran fallback;
- resultados host/build/hardware por separado.

La documentación debe poder permitir repetir el trabajo **sin el APK**, pero nunca sustituir evidencia de hardware cuando se cambia el runtime Vita.
