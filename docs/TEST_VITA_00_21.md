# Prueba Vita 00.21 — cierre antes del menú

> **Historical document notice — current v1.0 contract:** this file preserves
> evidence/instructions for the build or investigation named here. The current
> Vita runtime uses only `ux0:data/DBTapBattle/profiles/<Profile>/`; it has no
> current `game/` or `mods/` profile roots and no built-in Original selector
> row. Do not reuse historical install paths for v1.0. See
> [CURRENT_RUNTIME_CONTRACT](CURRENT_RUNTIME_CONTRACT.md).


**Prueba archivada; resultado físico recibido al 2026-10-05.** VPK probado:
`DBTapBattle-Vita-00.21-audio-startup-fix.vpk`, fuente
`07222bb42f20ab2bac953531e42b8cf3796940ca`. Esta actualización documental no
recompila el juego. Estado y próximas comprobaciones: [CURRENT_STATUS](CURRENT_STATUS.md).
Método/limitaciones de pruebas: [VALIDATION](VALIDATION.md); receta: [BUILD](BUILD.md).

00.20 no logra arrancar el worker de audio en tu prueba. Después de repetir el
intento 20 veces, el motor termina porque no puede cargar la música bgm_16.
El log confirma ese cierre controlado; no contiene un crash de PAC ni un código
que permita distinguir la creación del arranque del hilo.

00.21 restaura la prioridad que funcionaba en 00.19. Ahora registra el nombre
exacto de la operación de audio que falle y su código hexadecimal/decimal,
limpia sus recursos y evita repetir la misma inicialización fallida por cada
efecto. Conserva las cargas PAC selectivas, las cachés, el filtro de voces,
los textos y las mejoras gráficas anteriores.

Instala `DBTapBattle-Vita-00.21-audio-startup-fix.vpk` encima de la aplicación
actual con VitaShell. Conserva `ux0:data/DBTapBattle/` y los guardados de cada
perfil. El ZIP de símbolos solo sirve para diagnóstico; no se instala.

1. Selecciona Original y deja pasar la introducción: debe llegar al menú con audio.
2. Si llega, prueba cambios de personaje, voces y una pelea; comprueba también
   que los textos siguen visibles y la pelea conserva los 60 FPS.
3. Si se vuelve a cerrar, comparte `ux0:data/DBTapBattle/logs/runtime.log`, incluso
   si no aparece un psp2core. Esta versión debe identificar `00.21` y `07222bb`.

La compilación y las pruebas host no sustituyen tu comprobación en consola.
Se probaron la mezcla/DSP y las rutas de error reales del adaptador con APIs
Vita simuladas. La prueba posterior confirma worker y menú; luego rechaza char00 por el
filtro187 y sale con NullPointerException en md=1018/frame 1273. La calidad
audible y la pelea de esta versión no quedaron comprobadas. Nueva prueba:
[TEST_VITA_00_22](TEST_VITA_00_22.md), [evidencia](evidence/vita_hardware_selection_00.21.json).
Evidencia del cierre: `evidence/vita_hardware_audio_startup_00.20.json`.
El motor completo se compiló a ELF/VELF/SELF/VPK; se verificaron CRC, versión,
título, eboot y fuente. Hashes: `evidence/vita_audio_startup_build_00.21.json`.


## Identificar la prueba y conservar evidencia

- VPK: 2.235.382 bytes, SHA-256
  `ec551654abc68dfc5494c4f6b22ca620727c8fb898ff76b1682bbf141554bc47`.
- Versión SFO: 00.21; TITLE_ID: DBTB00001; CRC, eboot y fuente comprobados.
- Anota perfil y SHA del APK desde su dbtb_manifest.json: Original es el nombre
  de una carpeta, no prueba de que sea el primer APK ni Gen.
- Guarda el runtime.log de cada sesión antes de iniciar otra. Si hay cierre,
  registra la última pantalla y la operación/código de audio; si existe un
  psp2core, consérvalo junto al log.
- Para comparar personajes usa un perfil con tripletas completas. Anota primera
  visita, visita repetida, personaje/frase y resultado audible; una grabación
  ayuda a correlacionar el ruido con las mediciones.

Los probes host ejecutan el adaptador real y prueban fallos inyectados de abrir
puerto/crear hilo/arrancar hilo, limpieza y recuperación tras Dispose. Las APIs
Vita están simuladas: no prueban aceptación de prioridad por el kernel, salida
física, calidad de voz ni FPS. El probe de recursos usa PNG real y GL simulado.
Solo la prueba en consola puede cerrar esos pendientes.

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current hardware checkpoint — 00.34 (2026-10-07):** the exact
> `DBTapBattle-Vita-00.34-Button-Text-Center-Fix.vpk` is user-confirmed stable
> and functional on physical PS Vita for the exercised selector, profile-loading
> and gameplay paths, with no issue found so far. It retains the 00.33
> protected-PAC ownership fix and uses the unified `profiles-v1` data contract.
> See [CURRENT_STATUS](CURRENT_STATUS.md).
<!-- DBTB_CURRENT_CHECKPOINT:END -->
