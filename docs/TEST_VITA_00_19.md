# Prueba Vita 00.19 — texto y saturación de audio

00.18 queda confirmado a 60 FPS en pelea por tu prueba y el runtime.log.
Esta actualización conserva los cambios gráficos y corrige dos puntos:

- Restaura la consulta PVF del tamaño de imagen de cada letra; mantiene la
  fuente en memoria, la caché y la generación solo de glifos visibles.
- Evita el recorte de picos al sumar voces, música y efectos mediante un
  limitador estéreo con recuperación gradual. Conserva velocidad/tono original.

Algunas voces del gen.apk suministrado ya tienen muestras al límite de PCM16.
El limitador controla la saturación añadida por la mezcla; no reconstruye audio
que venga distorsionado en el archivo. El resultado audible en Vita sigue
pendiente de prueba. Las pausas por cargar recursos al cambiar personaje siguen
siendo un problema distinto y no se declaran resueltas en esta versión.

## Instalar y comprobar

Instala `DBTapBattle-Vita-00.19-text-audio-test.vpk` encima de la aplicación actual
con VitaShell. Conserva `ux0:data/DBTapBattle/` y los save.bin de cada perfil.
Los símbolos ZIP son para diagnóstico; no se instalan en la consola.

1. Usa el mismo perfil y comprueba nombres, menús y descripciones de personajes.
2. Escucha las mismas voces que antes sonaban roncas, en selección y en pelea.
3. Comprueba que pelea siga a 60 FPS, también con golpes y efectos.
4. Cierra el juego y comparte `ux0:data/DBTapBattle/logs/runtime.log` si persiste
   algún fallo. Para ruido persistente, incluye personaje/frase y una grabación
   breve: el log puede medir saturación y tiempo, pero no lo que se escucha.

El log de 00.19 identifica `bb78269`. Los primeros glifos registran `PVF glyph`
con rectángulo y cantidad de píxeles. `Voice PCM` registra frecuencia, pico y
muestras de origen al límite. `audio_overload_samples` cuenta picos de la suma
antes del limitador; `audio_clip_samples` cuenta recorte real después de este.
`audio_late_mix` sigue midiendo solo el tiempo de cálculo del bloque de mezcla,
no todos los posibles problemas del controlador o salida de audio.

## Validación realizada

Motor original completo compilado a ELF/VELF/SELF/VPK 00.19 (no el smoke vacío).
Pruebas de texto y audio con ASan/UBSan, 198 RIFFs reales con muestras conservadas,
12 regresiones Python, prueba JVM de buffers y CI nativo correctos. Metadatos y
hashes del artefacto están en `evidence/vita_text_audio_build_00.19.json`.
Texto y calidad audible todavía necesitan confirmación en hardware.

Probe de texto, usando stubs exclusivamente para PVF/GL:

```sh
g++ -std=c++14 -O2 -fsanitize=address,undefined -Itests/text_stubs \
  -Itools/aot/engine/native -Isrc tests/test_vita_text.cpp -o /tmp/dbtb-text-probe
ASAN_OPTIONS=detect_leaks=0 /tmp/dbtb-text-probe
```
El probe de audio conserva el comando de `TEST_VITA_00_18.md` y añade verificaciones
del limitador, forma/estéreo, recuperación, RIFF con metadatos y datos truncados.
