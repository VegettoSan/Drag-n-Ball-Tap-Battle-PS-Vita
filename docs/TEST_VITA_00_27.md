# Prueba física PS Vita — 00.27 Samu Direct Audio

## Objetivo

Validar en hardware que el perfil `DragonBallZuperSamuGamerYT.apk` funciona
con sus recursos **tal cual vienen en el APK**, incluyendo los BGM que conservan
extensión `.ogg` pero cuyo contenido real es MP3 o AAC/M4A.

No convertir, recodificar ni renombrar ningún BGM para esta prueba.

## Identidad exacta

- APP_VER: `00.27`
- TITLE_ID: `DBTB00001`
- Runtime source marker: `926eb6`
- VPK: `DBTapBattle-Vita-00.27-Samu-DirectAudio-Test.vpk`
- VPK SHA-256:
  `aed6da94abb44e8ee1cf8f889aa72b674a4422d506422dc5b074390ff500a6bf`
- eboot SHA-256:
  `0abcd48953c61692c19522c4cd68f897cd41e2e74d9ee39bb5d2303bf1f32b2f`
- ELF SHA-256:
  `079bdda19f5deb4f579fac8677e428c3f691bd89376eb3f3b7a13bc7d5a0f1e8`
- TeaVM: 467 clases / 4086 métodos
- LiveArea: PASS
- Build funcional interactivo: generated TeaVM `-O0`, adaptadores nativos `-O2`.

00.24 sigue siendo el último checkpoint confirmado físicamente. 00.27 es un
candidato de prueba y no debe promoverse hasta completar esta matriz.

## Preparar Samu sin modificar assets

Fuente auditada:

```text
DragonBallZuperSamuGamerYT.apk
SHA-256:
1771d71de25d664894dfb33b4a296ad6d30f5d897a34eb1ec14f3135496ec41d
```

Ruta reproducible:

```sh
python3 tools/prepare_samu_mod.py \
  DragonBallZuperSamuGamerYT.apk \
  ./install/mods/ZuperSamu
```

El helper solo valida y extrae. El manifest debe indicar
`payloads_unchanged: true`.

Copiar el resultado a:

```text
ux0:data/DBTapBattle/mods/ZuperSamu/
```

Conservar `ux0:data/DBTapBattle/game/` para fallback. No usar el ZIP AudioFix
de 00.26; ese camino de conversión está obsoleto.

## Qué debe ocurrir con el audio

El juego sigue solicitando los mismos nombres `bgm_XX.ogg`.

- `bgm_12` y `bgm_13`: Vorbis original → libvorbisfile.
- 12 pistas reales MP3 → decoder MP3 de Vita `SceAudiodec`.
- `bgm_09`, `bgm_10`, `bgm_11`: AAC-LC dentro de M4A → demux ISO-BMFF +
  decoder AAC de Vita `SceAudiodec`.
- SE y voces conservan sus caminos ya existentes.

En `runtime.log`, una pista MP3/AAC aceptada debe producir una línea semejante a:

```text
Compressed BGM direct: bgm_XX.ogg codec=mp3 ...
```

o:

```text
Compressed BGM direct: bgm_XX.ogg codec=aac-m4a ...
```

No debe aparecer un requisito de FFmpeg ni archivos `.ogg` convertidos.

## Matriz mínima

### Regresión base

Antes de concentrarse en Samu:

- iniciar la aplicación;
- confirmar LiveArea/menu;
- comprobar texto;
- comprobar SE y voces sin ronquido;
- abrir/cerrar cartas;
- iniciar una pelea y volver al menú.

### Roster Samu

Recorrer al menos:

```text
00 12 13 20 21 35 42 54 79 83 91
```

Iniciar pelea real con al menos:

```text
13 20 35 42 54 79 91
```

Los índices 35/54 cubren metadata `u`; 42 cubre la entry `.pn`; 20/21 cubren
`charf` vacíos pero válidos; 79/91 ejercitan la zona alta del roster.

### BGM directos

Intentar escuchar pistas representativas de las tres rutas:

- MP3: cualquier escenario/ruta que active `bgm_00`, y al menos una pista del
  grupo `03..08/14..15` si el flujo del juego permite alcanzarla;
- AAC/M4A: `bgm_09`, `bgm_10` y `bgm_11` si pueden alcanzarse;
- Vorbis: `bgm_12` o `bgm_13`.

Para cada una observar:

- que empieza a sonar;
- que no hay cierre/crash;
- que no suena acelerada/lenta;
- que los canales no están corruptos;
- que el loop no produce silencio permanente;
- que cambiar de pantalla/pista no deja el audio anterior colgado.

## Qué enviar si falla

Indicar personaje, pantalla, pelea y BGM aproximado cuando sea posible. Adjuntar:

- `ux0:data/DBTapBattle/logs/runtime.log` de esa sesión;
- `psp2core-*.psp2dmp` si Vita genera uno;
- video si el fallo es audible;
- si el juego sigue funcionando pero una pista queda muda, enviar igualmente el
  log: los mensajes `Compressed BGM rejected` permiten separar demux, decoder y
  mixer.

## Criterio de aceptación

00.27 puede considerarse hardware-confirmado para Samu cuando:

1. el roster alto llega a selección y pelea sin regresiones;
2. voces y SE siguen correctos;
3. al menos un MP3 y un AAC/M4A originales suenan mediante la ruta directa;
4. Vorbis sigue funcionando;
5. no hay conversión previa de los archivos;
6. iniciar/terminar varias peleas y volver al menú no produce crash;
7. el log no muestra fallos persistentes de `SceAudiodec`.

Evidencia de build/host:
[evidence/vita_samu_direct_audio_00.27.json](evidence/vita_samu_direct_audio_00.27.json).
