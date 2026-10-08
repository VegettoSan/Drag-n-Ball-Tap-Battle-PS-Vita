# Instalación y extracción de datos — Dragon Ball Tap Battle Vita v1.1

Esta guía corresponde al **candidato de publicación v1.1 (Universal Mod Support)**, `TITLE_ID DBTB01178`, Vita `APP_VER 01.01`. Se ha probado en PS Vita real la ejecución y las peleas consecutivas del mod `dbz mobile v9`, además de las pruebas históricas del juego original y otros mods. **No se garantiza que funcionen todos los mods existentes.**

## 1. Instalar o actualizar el VPK

1. Transfiere `Dragon-Ball-Tap-Battle-PS-Vita-v1.1.vpk` a PS Vita.
2. Instálalo con **VitaShell**, encima de la versión instalada. **No desinstales primero** el juego ni borres sus datos.
3. El `TITLE_ID` permanece `DBTB01178`; el ejecutable es idéntico al VPK **VisualQuality** probado por el usuario. Solo `sce_sys/param.sfo` pasó de `APP_VER 01.00` a `01.01` para esta publicación.
4. Los perfiles y las partidas existentes permanecen en `ux0:data/DBTapBattle/profiles/`. Aun así, es recomendable hacer una copia de seguridad antes de actualizar.

**El VPK no incluye los APK ni los datos propietarios del juego.** Debes extraerlos a partir de una copia de APK que tengas legalmente.

## 2A. Extraer desde Windows

1. Descarga `DBTapBattle-Extractor-Windows-v1.1.zip` de la misma release.
2. Descomprime **todos** sus archivos en una misma carpeta, incluidos `PrivateModDex.ps1` y `Extraer_APK_para_Vita.ps1`.
3. Arrastra uno o más APK de Dragon Ball Tap Battle sobre `Extract_APK_for_Vita.bat` (o sobre `Extraer_APK_para_Vita.bat`). También puedes ejecutar el BAT normalmente y elegir el APK.
4. Espera a que termine la validación. La herramienta creará una salida dentro de `Listo_para_Vita/`, con una carpeta `data/` lista para copiar.
5. Con VitaShell (FTP o USB), copia **la carpeta `data` del resultado a la raíz de `ux0:`**, combinando carpetas cuando corresponda.

Funciona en Windows 10/11 con PowerShell 5.1 incluido. No requiere Java, Python, 7-Zip ni conexión a Internet durante la extracción.

## 2B. Extraer desde Android, PC o navegador Web

1. Abre **https://vegettosan.github.io/Drag-n-Ball-Tap-Battle-PS-Vita/**.
2. Toca **SELECT APK** y elige tu APK; después **EXTRACT DATA FOR PS VITA**.
3. Espera al final del proceso y usa **DOWNLOAD ZIP**.
4. Descomprime el ZIP generado y copia su carpeta **`data/`** a la raíz de **`ux0:`** mediante VitaShell/FTP/USB. Si lo descargas en un teléfono, puedes transferir el ZIP al PC o extraerlo desde el propio teléfono antes de copiar.
5. Comprueba la ubicación final indicada abajo.

**Privacidad:** el APK se procesa localmente en el navegador; el extractor Web no lo sube a un servidor. La extracción de APK muy grandes puede requerir bastante memoria del navegador; en ese caso utiliza la versión Windows.

## 3. Comprobar la ruta en Vita

La ruta correcta es:

```text
ux0:
└── data/
    └── DBTapBattle/
        └── profiles/
            ├── gen/
            │   ├── common.pac
            │   ├── ...
            │   └── dbtb_manifest.json
            └── dbz_mobile_v9/
                ├── common.pac
                ├── char00.pac
                ├── ...
                ├── dbtb_manifest.json
                └── dbtb_codec.json   (solo cuando lo genera el extractor)
```

Cada APK se convierte en un perfil independiente, cuyo nombre procede del archivo APK. Para ciertos mods PRIVATE desconocidos, **los extractores generan automáticamente `dbtb_codec.json`**; debes copiarlo con los PAC. Los perfiles antiguos pueden funcionar sin él. No hay que editar manualmente las claves.

**Ruta incorrecta:** `ux0:data/data/DBTapBattle/`. No copies el APK ni el ZIP directamente dentro de `profiles/`.

## 4. Guardados y compatibilidad

Cada perfil almacena el progreso en `ux0:data/DBTapBattle/profiles/<Perfil>/save.bin`. Al actualizar el VPK, no es necesario reextraer los perfiles existentes y los guardados no deberían alterarse. Haz una copia de seguridad si vas a sustituir manualmente carpetas.

El lector PRIVATE recupera parámetros del APK, pero **no ejecuta código Android modificado**: un mod que cambie mecánicas mediante DEX puede no ser compatible aunque sus archivos se extraigan correctamente.

### Limitación de memoria y nitidez (importante)

Algunos mods tienen **atlas de sprites, fondos, efectos o imágenes de resolución muy alta**. Su carga completa puede superar las posibilidades de memoria del sistema/GPU de PS Vita y provocar `std::bad_alloc`, cuelgues o cierres. La v1.1 dispone de una **política genérica de seguridad**, aplicada por características de los recursos y presión de memoria (sin lista de excepciones por mod), que conserva la decodificación del archivo fuente pero puede **reducir la resolución física de ciertas texturas**, además de utilizar almacenamiento gráfico compacto. Por eso algunos elementos pueden verse **borrosos** y otros permanecer nítidos. Es una decisión deliberada para mantener las peleas estables. La reducción no cambia el APK ni reescribe los PAC extraídos.

Los tiempos de carga de estos mods pueden ser superiores a los del original o mods livianos.

## 5. Reportar problemas

Abre un [GitHub Issue](https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/issues) e indica: versión del VPK, nombre/version del APK/mod, acción y personajes usados, qué esperabas y qué sucedió, si falló en la primera o siguiente pelea; adjunta `ux0:data/DBTapBattle/runtime.log` (o la ubicación del log que tengas configurada), y el archivo `psp2core*.psp2dmp` si hubo crash. No publiques APK ni datos comerciales en el repositorio.

Más información técnica: [Compatibilidad de mods](MODS.md), [estado del port](CURRENT_STATUS.md), [informe de memoria del mod v9](DBZ_MOBILE_V9_COMBAT_OOM_2026-10-07.md), [extractor Web](WEB_DATA_TOOL.md) y [extractor Windows](WINDOWS_DATA_TOOL.md).
