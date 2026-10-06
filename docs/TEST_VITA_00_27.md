# Prueba física PS Vita — 00.27 Samu + Invasion Direct Audio

## Objetivo

Validar con el mismo ejecutable 00.27 dos rutas complementarias de mods:

1. **Zuper/SamuGamerYT:** roster masivo de 92 personajes con PAC ordinarios y
   BGM MP3/AAC/Vorbis conservados tal cual vienen en el APK.
2. **TAP BATTLE INVASION BETA 3:** roster extendido de 22 personajes, PAC
   protegidos del perfil Invasion y siete BGM no-Vorbis reproducidos directamente.

No convertir, recodificar ni renombrar ningún BGM para estas pruebas. El backend
00.27 detecta el codec por contenido después de que el VFS resuelve el perfil
seleccionado.

## Identidad exacta

- APP_VER: `00.27`
- TITLE_ID: `DBTB00001`
- Runtime source marker: `926eb6`
- VPK binario probado: `DBTapBattle-Vita-00.27-Samu-DirectAudio-Test.vpk`
- VPK SHA-256:
  `bb13580e6092076d5acca9e9de9cac4b7081e09aeecfcf2761217f3344ebc030`
- eboot SHA-256:
  `447324fcd3c0c6400f7a3c3cea92bc3a105f64c240831e376f289a535341a5a3`
- ELF SHA-256:
  `ecb70e9686b70a330dad4b85e1d0448791ff67a2d1b238c24d1610d1c46704f3`
- TeaVM: 467 clases / 4086 métodos
- LiveArea: PASS
- Build funcional interactivo: generated TeaVM `-O0`, adaptadores nativos `-O2`.

El nombre histórico del VPK contiene “Samu”, pero el ejecutable no codifica un
perfil Samu específico: la ruta MP3/AAC es genérica por contenido y el mismo
binario debe usarse para Invasion. 00.24 sigue siendo el último checkpoint
confirmado físicamente; 00.27 es candidato hasta completar estas matrices.

## Preparar Samu sin modificar assets

Fuente auditada:

```text
DragonBallZuperSamuGamerYT.apk
SHA-256:
1771d71de25d664894dfb33b4a296ad6d30f5d897a34eb1ec14f3135496ec41d
```

```sh
python3 tools/prepare_samu_mod.py \
  DragonBallZuperSamuGamerYT.apk \
  ./install/mods/ZuperSamu
```

El helper valida y extrae. Debe quedar `payloads_unchanged: true`.

Copiar a:

```text
ux0:data/DBTapBattle/mods/ZuperSamu/
```

No usar el AudioFix de 00.26.

## Preparar Invasion sin modificar assets

Fuente auditada:

```text
TAP BATTLE INVASION BETA 3.apk
SHA-256:
caaf294ddb9bf833868d7b541fc310603827bed44072230f60e0552cbb2dc94d
```

```sh
python3 tools/prepare_invasion_mod.py \
  "TAP BATTLE INVASION BETA 3.apk" \
  ./install/mods/Invasion
```

El helper usa el extractor Community14 existente para canonicalizar únicamente
los aliases protegidos. Los bytes de los 177 assets no se transcodifican y el
manifest debe indicar:

```text
pac_codec: community14-invasion-05aa0c5e
payloads_unchanged: true
```

Copiar a:

```text
ux0:data/DBTapBattle/mods/Invasion/
```

Mantener `ux0:data/DBTapBattle/game/` durante esta matriz porque el port
usa un modelo overlay + fallback para regresión segura. Sin embargo, Invasion
**no está incompleto** por omitir `bobj00.pac` y `font00.pac`: los APK
protegidos auditados son autónomos con ese inventario. El auditor Vita ya no
exige `bobj00.pac`; si el core original lo solicita en alguna ruta concreta,
el VFS puede resolver una copia base como compatibilidad adicional.

## Matriz de audio esperada

### Samu

- 12 MP3 directos por `SceAudiodec`.
- 3 AAC-LC/M4A directos por demux ISO-BMFF + `SceAudiodec`.
- 2 Vorbis por libvorbisfile.
- 19 SE y voces por sus rutas existentes.

### Invasion

- `bgm_03`: MP3 44.1 kHz.
- `bgm_04`: AAC-LC/M4A 44.1 kHz.
- `bgm_05`: AAC-LC/M4A 44.1 kHz.
- `bgm_06`: MP3 48 kHz.
- `bgm_07`: MP3 48 kHz con ID3.
- `bgm_14`: MP3 44.1 kHz.
- `bgm_15`: MP3 44.1 kHz con ID3.
- los otros 10 BGM: Vorbis.
- los 19 SE: Vorbis baseline.

Los AAC de Invasion tienen access units máximas de 455 y 548 bytes, por debajo
de `SCE_AUDIODEC_AAC_MAX_ES_SIZE=1536`.

Un MP3/AAC aceptado debe registrar:

```text
Compressed BGM direct: bgm_XX.ogg codec=mp3 ...
```

o:

```text
Compressed BGM direct: bgm_XX.ogg codec=aac-m4a ...
```

## Matriz mínima de prueba

### 1. Regresión base

Antes de cada perfil:

- iniciar la aplicación y comprobar LiveArea/menu;
- texto correcto;
- SE y voces sin ronquido;
- abrir/cerrar cartas;
- iniciar pelea;
- volver al menú.

### 2. Samu — roster masivo

Recorrer:

```text
00 12 13 20 21 35 42 54 79 83 91
```

Pelear al menos con:

```text
13 20 35 42 54 79 91
```

35/54 cubren metadata `u`; 42 cubre `.pn`; 20/21 cubren `charf` vacíos
válidos; 79/91 ejercitan la zona alta del roster.

Para audio, intentar alcanzar al menos un MP3, un AAC/M4A y un Vorbis.

### 3. Invasion — roster protegido extendido

Recorrer como mínimo:

```text
00 12 13 15 20 21
```

Pelear obligatoriamente con al menos:

```text
13 15 20 21
```

El 15 es especialmente útil porque su `char15.pac` tiene 101 entries y una
RGBA de 736x500; 20/21 ejercitan los layouts grandes del final del roster.

Para audio, priorizar si el flujo permite alcanzarlos:

```text
MP3:     bgm_03, bgm_06 o bgm_07
AAC/M4A: bgm_04 o bgm_05
Vorbis:  cualquier BGM no listado como especial
```

`bgm_05` es la mejor prueba de streaming/decodificación AAC porque es la pista
larga (~195 s), sin requerir PCM completo en memoria.

### 4. Churn

Con cada perfil:

- cambiar varios personajes repetidamente;
- iniciar al menos dos peleas distintas;
- volver al menú entre ellas;
- cambiar de BGM/pantalla;
- comprobar que no queda una pista anterior colgada;
- comprobar que no reaparece `bad_alloc`.

## Qué enviar si falla

Indicar primero el perfil: **ZuperSamu** o **Invasion**. Luego personaje,
pantalla/pelea y BGM aproximado si aplica. Adjuntar:

- `ux0:data/DBTapBattle/logs/runtime.log` de esa sesión;
- `psp2core-*.psp2dmp` si Vita genera uno;
- video si el fallo es audible o visual.

Si el juego sigue vivo pero un BGM queda mudo, enviar igualmente el log:
`Compressed BGM rejected` separa fallo de demux, decoder y mixer.

## Criterio de aceptación

### Samu

Hardware-confirmado cuando el roster alto llega a pelea, voces/SE siguen
correctos y al menos un MP3 + un AAC/M4A original suenan por la ruta directa.

### Invasion

Hardware-confirmado para **compatibilidad de recursos** cuando:

1. personajes 13..21 pueden seleccionarse y llegar a pelea;
2. al menos un MP3 y un AAC/M4A originales suenan directamente;
3. los PAC protegidos, textos, voces y SE no presentan regresión;
4. se puede volver al menú y comenzar otra pelea.

Esto no certifica automáticamente las mecánicas exclusivas de su `classes.dex`.
Si un personaje/recurso carga pero una mecánica concreta difiere de Android, ese
caso debe aislarse y compararse con el DEX de Invasion antes de tocar el core.

Evidencia de build/host:
[evidence/vita_samu_direct_audio_00.27.json](evidence/vita_samu_direct_audio_00.27.json).
