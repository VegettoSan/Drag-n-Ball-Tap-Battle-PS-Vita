# Prueba fisica PS Vita — 00.25 Community Mod Profiles

## Identidad exacta del candidato

- Version Vita: `00.25`
- TITLE_ID: `DBTB00001`
- Runtime fuente: `d186dc65` (los commits posteriores de esta tanda solo agregan pruebas/CI/documentacion)
- VPK de prueba: `DBTapBattle-Vita-00.25-Mod-Profiles-Test.vpk`
- SHA-256 VPK: `39265deebeec6ff7954dc85cd2fd18300fa6de8cd546176e3413b9d2823c3173`
- SHA-256 eboot: `a1c35f53070a600edabb310c53f95dea269b637206461e5a7e8370839fc5f380`
- SHA-256 ELF: `c624a1f121a58da10d3d6bc481416d78a1799352c00b6c4d8c21a777ed93c591`
- TeaVM original: 467 clases / 4086 metodos.
- LiveArea: validacion exacta PASS; conserva los assets aprobados.
- El VPK contiene solo ejecutable, SFO, LiveArea y notices. No contiene APK ni
  datos comerciales.

**00.24 sigue siendo el ultimo checkpoint confirmado en hardware.** 00.25 es un
candidato nuevo y debe probarse antes de promoverlo.

## Datos probados en host

### Espanol Android14

- Perfil: `community14-es-d594affc`
- Extraccion real: 144 archivos; 106 PAC renombrados a nombres canonicos.
- Gate offline real: 13 tripletes de personajes completos.
- Normalizacion nativa real: 106 PAC, 361 imagenes protegidas, 198 WAV y 68 BIN.
- Resultado host: PASS.

### TAP BATTLE INVASION BETA 3

- Perfil: `community14-invasion-05aa0c5e`
- Extraccion real: 177 archivos; 139 PAC renombrados a nombres canonicos.
- Gate offline real: 22 tripletes de personajes completos (`00..21`).
- Normalizacion nativa real: 139 PAC, 731 imagenes protegidas, 344 WAV y 80 BIN.
- Resultado host: PASS.

Ambos mods omiten `bobj00.pac`. El VFS lo obtiene de
`ux0:data/DBTapBattle/game/bobj00.pac`. Por eso no borres la carpeta
`game/` original al copiar un mod.

El APK original sin datos descargados no forma por si solo una instalacion offline
completa: su extraccion base contiene 57 archivos y no los 13 tripletes de
personajes. Esto es esperado por el modelo original de descarga de datos.

## Instalacion

1. Conserva un backup de `ux0:data/DBTapBattle/`.
2. Instala `DBTapBattle-Vita-00.25-Mod-Profiles-Test.vpk` encima de la app actual.
3. No borres `ux0:data/DBTapBattle/game/`.
4. Para Espanol, copia el contenido preparado a
   `ux0:data/DBTapBattle/mods/Espanol/`.
5. Para Invasion, copia el contenido preparado a
   `ux0:data/DBTapBattle/mods/Invasion/`.
6. Inicia el juego y selecciona el dataset desde el selector de arranque.

## Matriz minima de prueba

### Regresion 00.24

Antes de concentrarte en los mods, confirma que una ruta que ya funcionaba en
00.24 sigue pasando:

- arranque y LiveArea;
- menu;
- sonido sin ronquido/distorsion;
- cambiar varios personajes sin pausa de 1–2 s;
- abrir cartas y salir normalmente;
- iniciar una pelea;
- jugar un rato y volver al menu.

Una regresion aqui tiene prioridad sobre cualquier problema especifico de mod.

### Espanol

- Selecciona `Espanol`.
- Confirma que entra al menu y que el texto traducido se muestra correctamente.
- Recorre varios de los 13 personajes.
- Abre la pantalla de cartas y vuelve.
- Inicia al menos dos peleas con personajes distintos.
- Comprueba voces, efectos y BGM.
- Vuelve al menu despues de una pelea.

### Invasion

- Selecciona `Invasion`.
- Confirma que entra al menu.
- Recorre personajes del rango original y tambien los agregados, especialmente
  indices equivalentes a `13..21`.
- Inicia peleas usando al menos un personaje agregado.
- Abre cartas y vuelve.
- Comprueba voces/efectos/BGM.
- Mantente el tiempo suficiente para ejercer BGM largos. `bgm_05.ogg` supera
  el limite PCM completo de 00.24 y en 00.25 usa streaming Vorbis acotado.
- Vuelve al menu y empieza otra pelea.

## Que enviar si algo falla

En cada reporte indica el perfil seleccionado (`Espanol` o `Invasion`) y el
paso exacto. Adjunta:

- `ux0:data/DBTapBattle/logs/runtime.log` correspondiente a esa sesion;
- el `psp2core-*.psp2dmp` solo si Vita genera uno;
- foto/video si el problema es visual o de audio;
- personaje, pantalla o pelea exactos donde ocurre.

No mezcles logs de pruebas anteriores: usa la sesion que empieza con el marker
de motor completo 00.25 / `d186dc65`.

## Criterio de promocion

No reemplazar el estado hardware-confirmado de 00.24 hasta que:

- la regresion base 00.24 pase en 00.25;
- Espanol llegue a pelea y vuelva a menu sin crash;
- Invasion pueda usar personajes agregados y llegar a pelea;
- no aparezca corrupcion de texto/imagenes/audio;
- no reaparezca el `bad_alloc` de BGM;
- el streaming de BGM largo no produzca cierres ni audio claramente roto.

Cambios propios del `classes.dex` de un mod que no existan en recursos pueden
requerir una adaptacion de gameplay adicional aunque todos los PAC carguen bien.
