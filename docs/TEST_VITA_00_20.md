# Prueba Vita 00.20 — carga PAC y voces

Tu prueba confirma que los textos regresaron en 00.19, pero las voces siguen
sonando mal y cambiar de personaje todavía causa pausas. 00.20 conserva el
renderizado de texto y las mejoras gráficas que llegaron a 60 FPS en 00.18.

El adaptador leía cada PAC entero antes de aplicar el filtro del motor original.
Ahora solo lee los bloques que ese filtro permite, conservando las entradas,
su orden y sus tipos. También reutiliza PACs, texturas inmutables y PCM de voces
con límites de memoria. Los archivos originales y los mods no se reescriben;
se conserva el fallback al perfil original y los guardados independientes.

Las voces mantienen los PCM16 originales y su frecuencia de 22050 Hz. La
conversión a 48000 Hz usa interpolación con filtro de reconstrucción en lugar
de la interpolación lineal anterior. El worker tiene mayor prioridad y registra
intervalos entre entregas al controlador. No se añade buffering de audio.

La primera visita todavía necesita leer, decodificar y subir recursos.
Las cachés son limitadas; una entrada expulsada necesita cargarse otra vez.
La latencia real y la calidad audible de esta versión están pendientes de tu
prueba en Vita. El cambio no permite prometer que toda pausa desaparezca.

## Instalar y comprobar

Instala `DBTapBattle-Vita-00.20-pac-voices-test.vpk` encima de la aplicación actual
con VitaShell. Conserva `ux0:data/DBTapBattle/` y los `save.bin` de cada perfil.
`DBTapBattle-Vita-00.20-symbols.zip` sirve para diagnóstico y no se instala.

1. Usa el mismo perfil y pasa por varios personajes. Vuelve a los anteriores
   para comparar la primera carga y la carga repetida.
2. Escucha una misma frase sin cambiar de personaje y después mientras navegas.
   Comprueba también las voces en pelea.
3. Comprueba que nombres y descripciones siguen visibles y que pelea conserva
   los 60 FPS durante golpes y efectos.
4. Si persiste un fallo, cierra el juego y comparte
   `ux0:data/DBTapBattle/logs/runtime.log`. Si las voces siguen mal, incluye
   personaje/frase y una grabación breve: el log no captura el sonido audible.

El arranque de este VPK identifica versión `00.20` y fuente `7f19f80`.
Las líneas de recurso muestran filtro, caché y bytes leídos. Los resúmenes
incluyen `resource_cache_hits`, `resource_io_KiB`, `texture_cache_hits` y
`voice_cache_hits`. `audio_submit_gaps` cuenta intervalos superiores a dos
bloques de audio entre entregas, y `audio_submit_max_us` registra el mayor.
Estos intervalos y `audio_late_mix` miden cosas distintas; un gap por sí solo
no prueba que el controlador haya agotado su cola de audio.

## Validación realizada

Motor original completo regenerado con TeaVM y compilado a ARM
ELF/VELF/SELF/VPK. ZIP, versión, título y coincidencia del eboot verificados.
Pruebas ASan/UBSan de lectura selectiva, cachés, texturas, remuestreo y mezcla
correctas; llamadas Vita/GL simuladas en las pruebas host.

En 26 PACs de personajes de los dos formatos, el filtro 33 redujo los bytes
solicitados al archivo de 91.081.701 a 11.707.264 (87%), con payloads seleccionados
idénticos. Esto mide I/O solicitado en host, no FPS ni tiempos en Vita.
El ensayo de una señal de 8 kHz redujo una imagen espectral de remuestreo
32,3 dB, conservando duración y el tono de prueba. Es evidencia del DSP;
la calidad audible y el costo del filtro en la consola siguen pendientes.

Regresión completa: 125 PACs, 137 contenedores, 470 texturas, 68 tablas BIN,
198 WAVs decodificados; además 12 pruebas Python y probe JVM de buffers.
Metadatos y hashes: `evidence/vita_pac_voice_build_00.20.json`.
