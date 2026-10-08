# Dragon Ball Tap Battle PS Vita — v1.1 Universal Mod Support

**Estado:** compilación aprobada por pruebas en PS Vita real; preparada para que el mantenedor publique manualmente la release.

**VPK:** `Dragon-Ball-Tap-Battle-PS-Vita-v1.1.vpk`  
**SHA-256:** `9953e8c99ce59a5b4b55dab3ae2caec788c39ffe1edb19a2d4e5833c958ee6bd`  
**Vita TITLE_ID:** `DBTB01178` · **APP_VER:** `01.01`  
**Extractor Windows:** `DBTapBattle-Extractor-Windows-v1.1.zip` · SHA-256 `148480f1f5447086796eaa66ad3f97a45af7a15b5c32f03eadea929b5c4d44a2`  
**Extractor Web:** https://vegettosan.github.io/Drag-n-Ball-Tap-Battle-PS-Vita/

## Novedades

- **Soporte ampliado para mods:** juego original y mods con datos compatibles, incluida detección de más mods DragonTap PRIVATE sin incorporar claves ni reglas específicas de cada APK al ejecutable.
- **Extractores Web y Windows:** generan perfiles independientes y recuperan automáticamente la estructura del APK. Los nuevos mods PRIVATE compatibles reciben `dbtb_codec.json`; los anteriores pueden no necesitarlo.
- **Mayor estabilidad:** reducidas las reservas de memoria de PAC, reproducción de música AAC/M4A por lectura incremental y políticas de carga compacta de texturas para evitar agotamiento de memoria al entrar y avanzar en combates.
- **Selector de perfiles, LiveArea, audio y guardados independientes** conservados de la versión anterior.
- **Compatibilidad progresiva:** la extracción correcta no garantiza que se pueda ejecutar cualquier cambio de lógica Dalvik/Android realizado por un mod.

## Compatibilidad validada en hardware

El usuario verificó en una PS Vita real que el mod **dbz mobile v9** abre, permite seleccionar personajes, iniciar y terminar una pelea y avanzar a varias peleas consecutivas, sin reproducir los crashes y bloqueos anteriores. `runtime.log` de la prueba final: selector con 7 perfiles, 78 mensajes `[TextureCompact]`, 7 pistas AAC indexadas, 3 límites de recursos de combate y largos tramos cercanos a 60 FPS. No contiene `std::bad_alloc` ni un crash registrado. La confirmación del usuario es la evidencia del resultado jugable; el log por sí solo no prueba compatibilidad universal.

Los perfiles originales y de mods previamente aceptados conservan su ruta histórica de carga. **No se han probado todos los mods disponibles**, por lo que se agradecen informes de compatibilidad.

## Limitación conocida: nitidez de algunas texturas

Algunos mods incluyen sprites, fondos, efectos y atlas de alta resolución de varios megabytes. La memoria del sistema y de la GPU de PS Vita es limitada; cargar todas esas imágenes a calidad máxima puede causar `std::bad_alloc`, cuelgues o cierres.

**Para priorizar la estabilidad, esta release usa formatos gráficos compactos y puede reducir dinámicamente la resolución de algunas texturas.** Por eso algunas imágenes pueden verse borrosas mientras otras siguen nítidas. Los tiempos de carga pueden ser mayores en mods pesados. No se modifica ni recomprime el contenido del APK/PAC durante la extracción, y la política de calidad es **general por formato, dimensiones y memoria**, no una lista de ajustes especiales por mod. La mejora de nitidez queda como trabajo futuro; esta publicación conserva la configuración que funcionó en la consola.

## Instalación resumida

1. Instala el **VPK v1.1** con VitaShell sobre la versión previa; **no desinstales** ni borres los perfiles.
2. **Windows:** descarga y descomprime el ZIP del extractor; arrastra el APK a `Extract_APK_for_Vita.bat`; espera a que termine.
3. **Web (Android/PC):** abre el [extractor Web](https://vegettosan.github.io/Drag-n-Ball-Tap-Battle-PS-Vita/), selecciona el APK, pulsa **EXTRACT DATA FOR PS VITA** y descarga el ZIP.
4. Descomprime la salida, y copia su carpeta `data/` en la raíz de `ux0:`, de modo que los archivos queden bajo `ux0:data/DBTapBattle/profiles/<Perfil>/`.
5. Abre el juego y elige el perfil. **Si existe `dbtb_codec.json`, cópialo junto con los PAC**. No es necesario volver a extraer los perfiles antiguos.

**Guía completa:** [Instalación y extracción para Windows/Web](INSTALLATION_AND_EXTRACTION.md).  
**Diagnóstico de resolución/memoria:** [Crash y solución experimental del mod v9](DBZ_MOBILE_V9_COMBAT_OOM_2026-10-07.md).

## Reportes

Si un mod no abre, presenta gráficos incorrectos, se congela o termina con crash, crea un [issue](https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/issues) con la versión del VPK, nombre del mod y pasos para reproducirlo; adjunta `runtime.log` y, si existe, el `psp2core*.psp2dmp`. No adjuntes ni publiques APK o archivos comerciales.

## Identidad y validación del paquete

La **v1.1 contiene el mismo `eboot.bin` que la compilación VisualQuality experimental aprobada en Vita** (SHA-256 `bea473a4f1287702eafb2fe79e1b529d192b862bcd1ef63d681bf13f6a6d3af2`). Solo se actualizó el valor `APP_VER` en `sce_sys/param.sfo`, de `01.00` a `01.01`. Se conservan el mismo `TITLE_ID DBTB01178`, 15 miembros, recursos del selector, LiveArea y seed de guardados. Comprobaciones: integridad ZIP/CRC y validador LiveArea **PASS**. La identidad 1.0 anterior permanece como histórico/respaldo.

El VPK y las herramientas no incluyen datos comerciales: utiliza archivos APK obtenidos legalmente y extráelos localmente.
