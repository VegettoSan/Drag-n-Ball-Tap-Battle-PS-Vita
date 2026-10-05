# Prueba Vita 00.21 — cierre antes del menú

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
Vita simuladas. La recuperación del menú y la calidad audible quedan pendientes.
Evidencia del cierre: `evidence/vita_hardware_audio_startup_00.20.json`.
El motor completo se compiló a ELF/VELF/SELF/VPK; se verificaron CRC, versión,
título, eboot y fuente. Hashes: `evidence/vita_audio_startup_build_00.21.json`.
