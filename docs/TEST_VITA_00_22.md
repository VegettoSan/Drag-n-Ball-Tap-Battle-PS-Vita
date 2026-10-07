# Prueba Vita 00.22 — cierre al escoger personaje

> **Historical document notice — current 00.34 contract:** this file preserves
> evidence/instructions for the build or investigation named here. The current
> Vita runtime uses only `ux0:data/DBTapBattle/profiles/<Profile>/`; it has no
> current `game/` or `mods/` profile roots and no built-in Original selector
> row. Do not reuse historical install paths for 00.34. See
> [CURRENT_RUNTIME_CONTRACT](CURRENT_RUNTIME_CONTRACT.md).


00.21 ya inicia el worker de audio y entra al menú en la prueba física recibida.
Al escoger personaje rechaza char00 por invalid GameData filter, luego sale con
NullPointerException en Game3, md=1018/frame 1273.

00.22 elimina la validación errónea 0..127: el juego original usa máscara 187
para metadatos/voces y 251 para solo metadatos. Se conservan sus bits de exclusión,
parser y tareas originales, junto con audio/texto/cachés anteriores.
La corrección pasa pruebas host; el resultado físico de 00.22 está pendiente.

## Instalar y probar

Instala `DBTapBattle-Vita-00.22-selection-filter-fix.vpk` sobre la aplicación
actual con VitaShell. Conserva ux0:data/DBTapBattle y las partidas de cada perfil.
El ZIP de símbolos es para diagnóstico; no se instala.

1. Abre el mismo perfil Original de 00.21 y llega al menú.
2. Escoge el primer personaje; comprueba nombre, descripción y que siga abierto.
3. Cambia entre varios personajes y vuelve a uno anterior, comparando primera
   visita y repetición. Prueba Android14 si está instalado, con base disponible.
4. Escucha las voces de selección y entra a una pelea: comprueba textos, cartas
   y FPS con efectos. Esta corrección no demuestra que las voces roncas o las
   pausas estén resueltas; anota personaje/frase y duración de la pausa.
5. Conserva `ux0:data/DBTapBattle/logs/runtime.log` antes de otro arranque. Si
   vuelve a cerrar, comparte el log y psp2core solo si se generó.

## Identidad y evidencia

- Fuente: `c40ce0a96a0effb129fe7dd2970a44ec6c3a7257`; log esperado: 00.22 / c40ce0a.
- VPK: 2,235,731 bytes; SHA-256:
  `96796359179cd99253e16d5ade6e4f33dd59ffcbd5b486d052c4325c3e99ec47`.
- SFO 00.22 / DBTB00001; ELF/VELF/SELF, CRC, eboot y motor original comprobados.
- [Fallo físico00.21](evidence/vita_hardware_selection_00.21.json).
- [Build y pruebas00.22](evidence/vita_selection_filter_build_00.22.json).

La regresión nueva falla en el código anterior con char00/filter 187. Con la
corrección pasan 26 PACs Gen/Community14 y máscaras187/251/altas/con signo, así
como copia/caché nativa, PNG real y el corpus 125 PAC/137 contenedores/470 texturas/
68 tablas/198 WAV. GL está simulado; no se ejecuta Vita en estas pruebas host.
La generación privada Java/C de 00.21 se conserva y se recompila completa; el
cambio es nativo. Perfil Original es una carpeta, no prueba de qué APK la pobló.
Guía de diagnóstico: [VALIDATION](VALIDATION.md), [CURRENT_STATUS](CURRENT_STATUS.md).

<!-- DBTB_00_22_RESOLUTION:START -->
## Resolution in 00.23

The battle-start memory failure recorded by this 00.22 test was retested after
restoring the original streaming PAC parser. The physical-Vita 00.23 session passed
the previously failing transition and gameplay proceeded normally with no error
observed. See [TEST_VITA_00_23](TEST_VITA_00_23.md).
<!-- DBTB_00_22_RESOLUTION:END -->

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current hardware checkpoint — 00.34 (2026-10-07):** the exact
> `DBTapBattle-Vita-00.34-Button-Text-Center-Fix.vpk` is user-confirmed stable
> and functional on physical PS Vita for the exercised selector, profile-loading
> and gameplay paths, with no issue found so far. It retains the 00.33
> protected-PAC ownership fix and uses the unified `profiles-v1` data contract.
> See [CURRENT_STATUS](CURRENT_STATUS.md).
<!-- DBTB_CURRENT_CHECKPOINT:END -->
