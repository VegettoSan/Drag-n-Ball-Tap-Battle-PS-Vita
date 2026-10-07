# Prueba física PS Vita — 00.26 Zuper/Samu large-roster candidate

<!-- DBTB_DOC_STATUS:START -->
> **Project checkpoint:** 00.33 is the current hardware-confirmed development
> checkpoint for the tested paths. This file may document an earlier component
> or build; see [CURRENT_STATUS](CURRENT_STATUS.md) for authoritative status.
<!-- DBTB_DOC_STATUS:END -->


## Estado

- APP_VER objetivo: `00.26`
- TITLE_ID: `DBTB00001`
- Base funcional que no se debe perder: 00.24 hardware-confirmado.
- 00.26 añade capacidad de auditoría para IDs de personaje `00..99` y la ruta
  reproducible de preparación del mod Samu.
- El APK Samu observado contiene 92 personajes reales: `00..91`.
- El core Java/TeaVM de gameplay no fue reemplazado ni parcheado para Samu.
- Este documento no convierte 00.26 en hardware-confirmado: hace falta esta prueba.

## Identidad exacta del VPK entregado

- Archivo: `DBTapBattle-Vita-00.26-Samu-Roster-Test.vpk`
- Bytes: `2,604,860`
- VPK SHA-256:
  `749b9d32e6ed62a7b4593cb6f0b5af6dc2cabbc97cd9f25986757700879e18f5`
- eboot SHA-256:
  `1de9962f19cf9c39a1534e9547de14e3a2569950a6b712ca49ab871443f90830`
- ELF SHA-256:
  `db580cd100ac330d88908a9db2cd71f53a50b70c2295700ea0a17fbba68e7e9e`
- Runtime source marker embebido: `d7a4aa2`
- Generación TeaVM privada: 467 clases / 4086 métodos.
- Símbolos comprobados en el ELF: `GameData.Init`, `TCBManajer.Game3`,
  `VitaEngine.main`, `dbtb_installedData`.
- LiveArea: PASS exacto en icon0, pic0, bg0, startup y template a1.
- Compilación interactiva: resto TeaVM `-O1`, `TCBManajer.c` `-O0`,
  adaptadores nativos Vita `-O2`.

El `-O0` de TCBManajer es solo una excepción de empaquetado del runner para
obtener el candidato físico; no corresponde a un cambio de lógica. No usar este
artefacto para afirmar rendimiento final hasta una build monolítica normal
`-O1`.

## Preparar el dataset Samu

Usar exactamente el APK auditado:

```text
DragonBallZuperSamuGamerYT.apk
SHA-256 1771d71de25d664894dfb33b4a296ad6d30f5d897a34eb1ec14f3135496ec41d
```

Con FFmpeg disponible:

```sh
python3 tools/prepare_samu_mod.py \
  DragonBallZuperSamuGamerYT.apk \
  ./install/mods/ZuperSamu
```

El preparador debe terminar indicando:

- 92 personajes `00..91`;
- namespace Vita `00..99`;
- 15 BGM MP3/AAC normalizados a Ogg Vorbis;
- 2 BGM Vorbis conservados;
- `dbtb_manifest.json` generado con hashes antes/después.

Copiar el contenido resultante a:

```text
ux0:data/DBTapBattle/mods/ZuperSamu/
```

No borrar `ux0:data/DBTapBattle/game/`. El VFS debe seguir pudiendo usar el
dataset original como fallback para cualquier recurso ausente.

## Regresión obligatoria antes de Samu

Antes de evaluar el mod, confirmar una ruta que ya funcionaba en 00.24:

1. arranque y LiveArea;
2. menú;
3. textos visibles;
4. voces sin ronquido/distorsión;
5. cambiar varios personajes sin pausa de 1–2 s;
6. cartas: entrar y salir;
7. iniciar una pelea;
8. jugar y volver al menú.

Una regresión de esta lista tiene prioridad sobre cualquier fallo específico de
Samu.

## Prueba del roster de 92 personajes

Seleccionar `ZuperSamu` en el selector de arranque.

Primero recorrer personajes de distintas zonas del roster, no solo los primeros:

```text
00  12  13  20  21  35  42  54  79  83  91
```

Motivo de esta matriz:

- 00/12: frontera del baseline Gen;
- 13: primer añadido respecto a Gen;
- 20/21: `charf0020/0021` son PAC vacíos válidos;
- 35/54: contienen metadata tipo `u`;
- 42: contiene la entry tipo `.pn` cuyo payload es PNG;
- 79/83: personajes altos con layouts no mínimos;
- 91: último personaje real del APK.

Para cada uno, observar:

- retrato/sprites;
- nombre/texto;
- animación de selección;
- voz;
- que cambiar al siguiente/anterior no congele ni cierre el juego.

Después iniciar peleas como mínimo con:

```text
00  13  20  35  42  54  79  91
```

Cada pelea debe llegar a gameplay real. Probar ataques, recibir daño, voces,
efectos, terminar o abandonar la pelea y regresar al menú.

## Prueba de audio Samu

El dataset preparado mantiene los nombres originales `bgm_00..16.ogg`, pero
los 15 que eran MP3/AAC ahora contienen Vorbis real. El ejecutable Vita debe
seguir usando su mismo backend libvorbisfile.

Intentar cubrir durante varias pantallas/peleas tantos BGM como sea posible,
prestando especial atención a:

```text
bgm_00
bgm_03
bgm_09
bgm_10
bgm_11
bgm_12
bgm_13
bgm_14
bgm_16
```

Los índices 09/10/11 eran AAC/M4A en el APK; 00/03/14/16 eran MP3; 12/13 ya
eran Vorbis. No debe aparecer `BGM load failed`, silencio inesperado, audio
ronco, cambio de velocidad ni crash por memoria al iniciar una pelea.

Las voces de personaje son independientes de esos BGM. Comprobar también voces
de personajes altos, especialmente 35, 54, 79 y 91.

## Qué enviar si falla

Adjuntar la misma sesión de prueba:

- `ux0:data/DBTapBattle/logs/runtime.log`;
- `psp2core-*.psp2dmp` si Vita genera uno;
- personaje exacto;
- pantalla/acción exacta;
- BGM si puede identificarse;
- foto/video si el problema es visual o de audio.

Si el fallo solo ocurre con un personaje, repetir una vez después de reiniciar
el juego y conservar ambos logs si difieren.

## Criterio para declarar Samu funcional

No promover el soporte Samu a hardware-confirmado hasta cumplir todo esto:

- regresión base 00.24 PASS;
- selector Samu entra al menú;
- roster puede recorrerse hasta el personaje 91 sin wrap a 00 ni cierre;
- los casos especiales 20/21, 35/54 y 42 no rompen selección;
- varios personajes añadidos y el 91 llegan a combate;
- voces siguen limpias;
- BGM antes MP3 y antes AAC reproducen después de la normalización;
- regreso a menú y segunda pelea funcionan;
- no aparece `bad_alloc`, `BGM load failed`, NPE de Game3 ni corrupción de
  recursos.

Hasta entonces, el estado correcto es **00.26 HOST/IMPORT CONFIRMED, hardware
pending**, manteniendo 00.24 como referencia física estable.
