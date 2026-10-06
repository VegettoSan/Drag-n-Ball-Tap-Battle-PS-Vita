# TAP BATTLE INVASION BETA 3 — referencia técnica completa

Fuente auditada: `TAP BATTLE INVASION BETA 3.apk`

- APK SHA-256: `caaf294ddb9bf833868d7b541fc310603827bed44072230f60e0552cbb2dc94d`
- APK bytes: 124,787,316
- entradas ZIP: 204
- `classes.dex` SHA-256: `05aa0c5ec839161e59b93eccd8657925380c56b46f1a4a452c662b1f115212d1`
- `classes.dex`: 701,740 bytes
- perfil PAC: `community14-invasion-05aa0c5e`

## Manifiesto

- package `rinne.private.apk`
- version `1.4` / code 7
- minSdk 8 / targetSdk 29
- `debuggable=true`
- launcher `.Primary` → `Lrinne/private/apk/Primary;`
- BluetoothSearch conserva namespace original
- mismos permisos no-billing del Android14 protegido.

Firma: certificado Android genérico, fingerprint SHA-256
`a40da80a59d170caa950cf15c18c454d47a39b26989d8b640ecd745ba71bf5dc`.

## Dataset

`assets/`: 177 archivos:

- 139 PAC
- 17 BGM
- 19 SE
- `loading.png`
- `mk.bin`.

No incluye `save.bin`, `bobj00.pac` ni `font00.pac`.

### Inventario lógico

- `back00..06` (7)
- `bobj01..07` (7)
- `card000..050` (51)
- `char00..21` (22)
- `chardemo00..21` (22)
- `charf0000..0021` (22)
- common/select/effect/demo/card_preview/gamedata/text00.

Respecto a Android14/Español añade exactamente 33 PAC:

- 9 char
- 9 chardemo
- 9 charf
- 3 back
- 3 bobj.

## Alias exactos

- `9036` → common
- `7E8F` → select0
- `1E1C` → effect
- `97E6` → demo_00
- `6E24` → demo_08
- `71DC` → font00 (alias observado en DEX; archivo no incluido)
- `0708` → card_preview
- `90EA` → gamedata
- `D37C` → text00
- `F813XX` → backXX
- `17A5XX` → bobjXX
- `0953XX` → charXX
- `364EXX` → chardemoXX
- `91F9XXXX` → charfXXXX
- `1A4BXXX` → cardXXX.

## Perfil binario

```text
count XOR       0x842F
offset XOR      0x71573ADB
size XOR        0x33AC6051
image width XOR 0xA42C
image height XOR 0xED15
table count XOR 0x68A3
table pos XOR   0x122E64CB
table width XOR 0xB1D9
table height XOR 0x7C00
WAV size XOR    0x842F
```

Claves observadas después de `raw_be32 ^ entry_index`:

- BIN `0xAEBFC3A0`
- CNV `0x877E379F`
- DAC `0x8125D853`
- RGBA `0xE7C20ECB`
- SPR `0x403D58E7`
- WAV `0x153CCB39`.

No se observó top-level ACT.

## Corpus PAC

139/139 PAC exteriores validan únicamente con este perfil. Contando nueve SPR anidados:

- RGBA 763 (731 top-level)
- WAV 344
- CNV 139
- DAC 139
- BIN 89 (80 top-level)
- SPR 9
- metadata desconocida 5.

Máxima textura observada:

- `char15.pac`
- 736×500
- 1,472,000 bytes RGBA decodificados.

Por tanto, cualquier guard `<=512` sería incorrecto para Invasion.

## Tablas y continuidad de personajes

- `gamedata`: 271 registros, 87,452 bytes normalizados, 39,829 celdas.
- `text00`: 1 registro, 24,970 bytes, 315 celdas.
- **cada uno de los 22 `charXX` mantiene 43 registros BIN**.
- las 51 cartas mantienen 1 registro BIN cada una.
- los 7 backgrounds mantienen 3 registros BIN cada uno.

La continuidad `00..21` y el mismo shape de 43 registros son evidencia fuerte de que los personajes adicionales siguen el mismo contrato de datos base. No prueban que toda la lógica del DEX de Invasion exista en el motor original.

## Outliers estructurales que un parser debe aceptar

Los 22 `charXX.pac` conservan un BIN convertido de 43 registros, pero **no**
comparten todos el mismo layout exterior. Los personajes 00..14 y varios nuevos
usan el patrón compacto habitual (RGBA iniciales, BIN/CNV/DAC, WAV finales), pero
hay excepciones importantes:

| PAC | Entries | WAV | Layout exterior resumido | Máxima RGBA |
|---|---:|---:|---|---|
| `char15.pac` | 101 | 19 | RGBA×19, BIN, luego RGBA intercaladas entre CNV/DAC/WAV y RGBA×39 finales | **736×500 @ entry 94** |
| `char20.pac` | 98 | 15 | RGBA×27, BIN, luego RGBA intercaladas entre CNV/DAC/WAV y RGBA×36 finales | 512×512 |
| `char21.pac` | 85 | 15 | RGBA×67, BIN, CNV, DAC, WAV×15 | 512×512 |

También:

- `char16.pac` tiene 41 entries y 19 WAV;
- `char13.pac` tiene 40 entries y 18 WAV;
- el resto de los personajes usan 15 o 18 WAV según el personaje;
- el BIN de todos sigue siendo de **43 registros**.

Por tanto, un parser/normalizador **no puede** asumir que todas las imágenes
están contiguas al inicio ni que todos los WAV están necesariamente al final.
Debe obedecer el directorio PAC y el tipo de cada entry por índice.

### Card034

`card034.pac` contiene un BIN convertido válido de un registro, pero ese
registro tiene dimensión 0×0 (0 celdas; payload de tabla 10 bytes). El mismo
caso existe en el APK Español y se conserva en Invasion. No debe rechazarse
simplemente porque `width * height == 0`; es un caso observado del corpus.

## DEX: mod de código real

Mismas dimensiones globales que Android14: 39 clases / 914 method IDs / 596 métodos con código. Sin embargo:

- 595 firmas con código son comunes con Android14.
- 223 conservan instrucciones idénticas.
- **372 cambian instrucciones**.
- cambia la clase launcher de `com.namcobandaigames.apk.Primary` a `rinne.private.apk.Primary`.

Distribución destacada de métodos modificados:

- `ext.C`: 136
- `ext.A`: 91
- `ext.q`: 14
- `BluetoothSearch`: 13
- `ext.E`: 9
- `ext.m`: 8
- múltiples cambios en `ext.d/n/w/F/c/e/u/y/...`.

El listado completo de firmas está en `docs/evidence/apk_deep_structure_2026-10-06.json`.

Consecuencia: Invasion es Tier A/B por datos **y Tier C por código**. No se puede prometer que todas sus mecánicas nuevas funcionen por cargar PACs.

## Linaje con el APK Español

Comparación plenamente decodificada de los 106 PAC compartidos:

- imágenes top-level: 303 iguales / 58 diferentes.
- PCM de voces: 195 iguales / 3 diferentes; las tres están en `char11`.
- tablas: 52 iguales / 18 diferentes.

Tablas cambiadas frente a Español:

- `card048`, `card049`, `card050`
- `char00..12`
- `gamedata`
- `text00`.

Esto, junto al texto español y al mismo helper nativo, indica una base de contenido muy cercana al APK Español. No atribuye autoría.

## Audio exterior — bloqueo importante

29 de 36 archivos son byte-idénticos al baseline Vorbis. Cambian siete BGM.

**Los siete conservan extensión `.ogg`, pero ninguno es Ogg Vorbis:**

| archivo | codec real | firma/contenedor | Hz | ch | duración | bitrate | bytes |
|---|---|---|---:|---:|---:|---:|---:|
| bgm_03.ogg | MP3 | MPEG frame `FF FB` | 44100 | 2 | 111.830 s | 256 kb/s | 3,578,566 |
| bgm_04.ogg | AAC | M4A/ISO BMFF `ftypM4A` | 44100 | 2 | 13.142 s | ~125 kb/s | 208,708 |
| bgm_05.ogg | AAC | M4A/ISO BMFF `ftypM4A` | 44100 | 2 | 195.419 s | ~125 kb/s | 3,087,225 |
| bgm_06.ogg | MP3 | MPEG frame `FF FB` | 48000 | 2 | 99.000 s | 256 kb/s | 3,168,000 |
| bgm_07.ogg | MP3 | ID3 + MPEG | 48000 | 2 | 127.104 s | 160 kb/s | 2,970,086 |
| bgm_14.ogg | MP3 | MPEG frame `FF FB` | 44100 | 2 | 138.188 s | 256 kb/s | 4,422,008 |
| bgm_15.ogg | MP3 | ID3 + MPEG | 44100 | 2 | 85.368 s | 160 kb/s | 1,707,930 |

Todos los 19 `se_XX` permanecen baseline Vorbis y byte-idénticos.

### Implicación Vita

El adapter actual de BGM abre por `ov_fopen()`. libvorbisfile no decodifica MP3 ni AAC/M4A. Por tanto:

- `bgm_03/04/05/06/07/14/15` fallarán en el camino Vorbis actual.
- el problema no es simplemente “BGM demasiado grande”.
- `bgm_05` sí equivaldría a ~32.87 MiB PCM16 stereo si se decodifica completo, pero primero debe existir soporte AAC.
- 00.25 no debe declararse audio-compatible con Invasion hasta adaptar esta frontera y probarla.

La solución correcta respeta el motor: el motor sigue solicitando `bgm_05`; el adapter Vita identifica el contenido real y lo reproduce/convierte. No cambiar reglas de juego ni nombres pedidos por el motor.

## `demo_00` especial

Entre los 106 PAC compartidos con Android14, 105 mantienen la misma secuencia top-level de tipos. `demo_00.pac` es la excepción:

Android14:

```text
RGBA ×6, CNV, DAC, unknown
```

Invasion:

```text
RGBA ×7, CNV, DAC, unknown
```

El parser no debe fijar un count esperado por nombre de PAC.

## Reglas para implementar soporte

1. no tocar el motor original para acomodar 22 personajes.
2. aceptar datos 00..21 solo mediante los contratos ya verificados y límites del adapter.
3. mantener fallback de `bobj00`/font desde base.
4. no asumir Vorbis por extensión.
5. no aumentar límites de memoria “a ciegas” para bgm_05; resolver primero codec y estrategia de streaming.
6. si una mecánica de personaje adicional falla, comparar la ruta concreta con los métodos DEX modificados; no portar 372 métodos indiscriminadamente.
7. documentar evidencia y regresión contra el VPK 00.24 estable antes de cambiar cualquier ruta ya funcional.
