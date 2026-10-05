# Prueba Vita 00.18 — personajes, voces y FPS

**Prueba archivada; resultado recibido.** El usuario confirma pelea estabilizada
a 60 FPS; el log registra 15 ventanas estables a 59,9 FPS. Los textos desaparecen,
las voces siguen roncas y persisten pausas de selección. El texto se recupera en
00.19. Las instrucciones siguientes conservan el protocolo de 00.18; para la
versión actual usa [TEST_VITA_00_22](TEST_VITA_00_22.md).
Evidencia: [prueba física 00.18](evidence/vita_hardware_performance_00.18.json).
Estado general: [CURRENT_STATUS](CURRENT_STATUS.md).

El chat anterior guardó 00.17 en main (interpolación de voces, reducción de llamadas
GL, reutilización de métricas PVF), pero no dejó un VPK de esa versión entregado.
Se recuperó además 1351457, con mezcla en porciones y cálculo Q16.
00.18 conserva esos cambios y añade:

- Fuente idéntica PVF cargada en memoria; vuelve al modo de archivo si PVF falla.
- Buffers gráficos reutilizados; posiciones/límites y orden original conservados.
- Menos divisiones/cálculos en la mezcla de audio; 22050 Hz, tono y tres canales
  de voces originales. Interpolación lineal hacia la salida de 48000 Hz.
- CPU 444, bus 166, GPU 222 y crossbar 166 MHz mediante API pública, con resultados
  efectivos registrados. Resolución 960x544.
- Diagnósticos por 120 frames en runtime.log para distinguir texto, carga,
  dibujo/Java, espera de presentación y saturación/coste de audio.
- Empaquetado ELF corregido y script privado compatible con el número de versión.

## Instalación histórica de 00.18

Instala DBTapBattle-Vita-00.18-performance-test.vpk encima del port actual con
VitaShell. Conserva las carpetas de datos y save.bin que ya usas; no hace falta
volver a extraer ni copiar APKs. Sigue habiendo un save independiente por perfil.

## Probar

1. Abre el mismo perfil usado en la prueba 00.16.
2. Cambia entre varios personajes; vuelve a visitar uno ya mostrado y compara
   la primera visita y la segunda. Comprueba texto japonés, nombres y descripciones.
3. Entra en pelea durante al menos un minuto. Anota FPS habituales y mínimos con
   golpes/efectos; escucha qué personaje/frase sigue sonando ronco, si ocurre.
4. Comprueba que cartas y arranque siguen funcionando; repite con Android14 o
   Original+Characters (gen.apk) si están instalados.
5. Cierra el juego y comparte ux0:data/DBTapBattle/logs/runtime.log. Un video corto
   con voz y FPS es útil si persiste el ruido. Si hay crash, incluye el psp2core.

## Evidencia

Build completo confirmado (motor original generado privadamente, no el VPK smoke).
Pruebas de buffers, mezclador ASan/UBSan, 12 regresiones Python, recursos Original/
Android14 y gen.apk superadas. Hash y commit del binario se registran en
`evidence/vita_performance_build_00.18.json`.

Al publicar esta hoja estaban pendientes la pausa, la voz y los FPS. El resultado
posterior confirma los FPS, conserva voz/pausas pendientes y detecta regresión
de texto. La evidencia de 00.18 no prueba esos resultados en 00.21. `audio_clip_samples` cuenta saturación al sumar fuentes;
`audio_late_mix` mide cálculo de mezcla fuera de plazo, no todos los posibles
underruns del driver. `swap_ms` incluye pacing y no prueba por sí solo saturación
GPU. No se han reemplazado reglas de combate ni reordenado sprites.

## Reproducir los probes

```sh
python tools/aot/engine/tests/run_gles_buffer_probe.py --ecj /tools/ecj-3.37.0.jar
# Requiere cabeceras/bibliotecas Vorbis de host; los stubs solo sustituyen Vita:
g++ -std=c++14 -O2 -fsanitize=address,undefined -Itests/audio_stubs \
  -Itools/aot/engine/native -Isrc tests/test_vita_audio.cpp src/vfs.cpp \
  -lvorbisfile -lvorbis -logg -o /tmp/dbtb-audio-probe
ASAN_OPTIONS=detect_leaks=0 /tmp/dbtb-audio-probe
python -m unittest discover -s tests -p 'test_*.py' -v
```

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current hardware checkpoint — 00.23 (2026-10-05):** build `00.23` from source
> commit `0e17b0ba` was tested on a real PS Vita. In the reported test path,
> startup/menu flow, text, audio/voices, character selection and entry into/playing
> a battle worked normally, with **no error observed in this session**. This makes
> 00.23 the current hardware checkpoint and resolves the 00.22 battle-start
> memory regression documented in the historical 00.22 records. Historical test
> documents remain historical evidence; this note does not claim exhaustive coverage
> of every character, mode, mod or long-duration session.
<!-- DBTB_CURRENT_CHECKPOINT:END -->
