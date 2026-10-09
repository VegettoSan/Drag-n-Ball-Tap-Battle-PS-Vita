# Compilar y publicar VPK desde GitHub Actions

<!-- DBTB_DOC_STATUS:START -->
> **Repository status — 2026-10-09:** main contains prepared v1.2 (`01.02` / `DBTB01178`),
> retaining v1.1 and approved Vita controls; dialogues use touch. Exact v1.2 hardware
> retest is pending; published release is 1.1. [Current contract](CURRENT_RUNTIME_CONTRACT.md) · [Status](CURRENT_STATUS.md).
> Current guide; explicitly dated experiments and superseded decisions remain historical.
<!-- DBTB_DOC_STATUS:END -->

La versión 1.2 se compiló localmente **sin ejecutar estos workflows**. Su
publicación manual usa la etiqueta `1.2`, siguiendo `1.1`; las etiquetas
automáticas de Actions conservan su esquema `v<APP_VER>`.

Hay dos botones manuales en **Actions**:

| Workflow | Etiqueta automatica (version actual) | Publicacion |
|---|---|---|
| [Publicar VPK - Release](https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/actions/workflows/vita-release.yml) | `v01.02` con el CMake actual | Release normal, marcada Latest |
| [Publicar VPK - Prerelease](https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/actions/workflows/vita-prerelease.yml) | `v01.02-pre.<run_id>` con el CMake actual | Prerelease para pruebas, no reemplaza Latest |

Ambos usan la version de `tools/aot/engine/vita/CMakeLists.txt`, el commit exacto
seleccionado al ejecutar el workflow y el mismo compilador compartido. No usan
el bootstrap de la raiz ni el ejecutable dummy del native smoke. La compilacion
de release tambien mantiene `-O1` para todo el core TeaVM y `-O2` para servicios
nativos, con las rutas frías y selector a -Os. La base de recursos VisualQuality
de v1.1 y los controles retenidos se aprobaron en pruebas separadas; el VPK
estable 1.2 recién compilado aún espera su resultado físico.

## Contrato de datos del VPK

El VPK v1.2 (APP_VER `01.02`, TITLE_ID `DBTB01178`) espera datos externos exclusivamente en:

```text
ux0:data/DBTapBattle/profiles/<Profile>/
```

La publicación del VPK no incluye esos datasets. Tanto Web Extractor 1.0 como
Windows Extractor 1.5 generan exactamente esa estructura y registran
`runtime_contract: profiles-v1`. La web está en
https://vegettosan.github.io/Drag-n-Ball-Tap-Battle-PS-Vita/ y procesa los APK
localmente en el navegador.

## Configuracion una sola vez

El runner de GitHub no puede acceder a los archivos de este chat. Necesita una
fuente privada del **APK original DBTapBattle.apk** usado para generar el core.
Android14 y Gen son datasets externos/mods; no sustituyen ese APK del compilador.

1. Dispon el APK original en una ubicacion privada con descarga directa HTTPS.
   La URL debe devolver el archivo, no una pagina HTML, y funcionar sin login
   interactivo. Si usa un enlace firmado, renuevalo cuando expire.
2. En el repositorio: **Settings → Secrets and variables → Actions → New repository
   secret**. Nombre: `DBTB_ORIGINAL_APK_URL`. Valor: esa URL privada de descarga.
3. Comprueba que las Actions esten habilitadas y que la politica del repositorio
   permita el permiso `contents: write` solicitado para publicar Releases.

El SHA-256 esperado esta fijado en el descargador:
`b84f98a3ed70957354f358b7930bd8fb651cc89b74e16f8774ebd989fbf0899b`.
Un APK distinto, una pagina de descarga, un enlace HTTP o un archivo corrupto
detienen el job. La URL se pasa como secreto, nunca como input visible ni en
argumentos de shell, logs, manifests o archivos publicados. No subas el APK al
Git publico para resolver este requisito.

## Uso

1. Abre **Actions → Publicar VPK - Prerelease** o **Publicar VPK - Release**.
2. Pulsa **Run workflow**, deja la rama **main** y, si quieres, escribe notas.
3. Ejecuta. El pipeline valida la fuente y el destino, prueba el audio, genera el
   core completo y compila ELF/SELF/VPK. Despues valida version, title ID,
   simbolos del juego, evidencia TeaVM, CRC, LiveArea y hashes.
4. La descarga aparecera en [Releases](https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/releases).

Una release normal usa `v<APP_VER>`. Si ya existe esa release o etiqueta, se
detiene sin sobrescribirla. Para una nueva version estable, cambia `VITA_VERSION`
en el CMake del **motor completo**, haz commit/push y ejecuta de nuevo. Cada nueva
ejecucion manual de prerelease tiene su propio run ID; reejecutar una publicacion
ya completada no reemplaza sus archivos. El sufijo prerelease identifica la
prueba en GitHub; el APP_VER del VPK sigue siendo la version base de CMake.

## Archivos publicados y conservacion de fuentes privadas

- `DBTapBattle-Vita-<version>[-pre.<run_id>].vpk`: juego completo, LiveArea, notices y los cuatro PNG validados del selector Gen.
- `DBTapBattle-Vita-<version>[-pre.<run_id>].symbols.zip`: solo ELF/VELF compilados
  para analizar futuros crash dumps; no incluye C/JAR/classes/APK.
- `build.json`: commit, version, canal, SHA del APK fuente, evidencia de clases/
  metodos, toolchain/digest Docker y hashes de los binarios.
- `SHA256SUMS.txt`: hashes del VPK, simbolos y manifest.

Los mismos archivos y las notas se conservan como artifact de Actions durante
14 dias. Los archivos de Releases permanecen disponibles. Los datos del juego,
APKs, JARs, clases, C generado y logs privados no se cachean ni se suben: estan
fuera del checkout, en un directorio temporal borrado incluso al fallar el job.
La entrada privada en un runner efimero es parte de estos workflows manuales
autorizados; los workflows automaticos siguen usando solo codigo/herramientas
publicas. No se distribuyen datasets de personajes/musica dentro del VPK. Desde
v1.0 se empaquetan unicamente cuatro PNG de interfaz derivados de Gen/select0.pac
para vestir el selector inicial; sus hashes son parte de la validacion de release.

La publicacion empieza como borrador con todos los archivos. Solo se hace
publica despues de verificar su commit, canal y subidas completas. Si GitHub
interrumpe una subida, puede quedar un borrador para revisar: no se publica
incompleto y el siguiente intento no lo sobrescribe automaticamente. Resuelve
ese borrador desde Releases o usa una nueva version/prerelease.

## Validacion y alcance

`validate-vita-publication.yml` valida automaticamente la sintaxis/expresiones
con actionlint y ejecuta las pruebas de publicacion, LiveArea y reconstruccion del selector. No usa el APK
privado ni publica. El builder manual ejecuta tambien las pruebas reales de
PCM de las 17 BGM del APK original y las pruebas ASan/UBSan de audio con APIs Vita
simuladas. Un fallo impide la publicacion. El ELF debe contener el motor original
y el log de generacion debe demostrar un core completo; un native link probe
no puede pasar por un VPK jugable.

00.34 es evidencia histórica de estabilidad. La base de recursos v1.1 y los
controles probados se conservan en v1.2. Cada VPK nuevo tiene otro commit/hash: compilar/publicar automáticamente no equivale a probar ese nuevo binario en consola. Usa [la prueba/resultados 00.34](TEST_VITA_00_34.md) como regresión histórica y guarda el VPK, `build.json`, `runtime.log` y el ZIP de símbolos correspondiente.

La sintaxis, pruebas unitarias y staging con el VPK real se verificaron localmente.
La [validacion en GitHub](https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita/actions/runs/37395626518)
tambien paso: actionlint y las 13 pruebas de publicacion/LiveArea, sobre el commit
`b17bb49ef1ab257ea74f68353a907b4f538c1c89`.
Una compilacion/publicacion completa en Actions necesita que el secreto
de descarga este configurado; no se ha simulado una publicacion real.

Referencias de implementacion: [workflows reutilizables](https://docs.github.com/en/actions/how-tos/reuse-automations/reuse-workflows),
[sintaxis y permisos](https://docs.github.com/en/actions/reference/workflows-and-actions/workflow-syntax),
[gh release create](https://cli.github.com/manual/gh_release_create) y
[gh release edit](https://cli.github.com/manual/gh_release_edit).


## Identidad de la primera release estable

La release **v1.0** usa APP_VER `01.00` y TITLE_ID `DBTB01178`. Hereda el
runtime/juego del checkpoint 00.34 confirmado en Vita física. Los documentos de
prueba 00.34 conservan `DBTB00001` porque describen el artefacto histórico exacto
que fue probado; no deben reescribirse como si ese VPK hubiera usado el nuevo ID.


## GitHub Pages / Web Extractor

`.github/workflows/pages.yml` publica el contenido estático de `web/` mediante
GitHub Pages. Antes del deploy:

1. ejecuta `node --check` sobre el núcleo y la UI;
2. ejecuta `tests/web_extractor_core.mjs`;
3. reconstruye y valida los cuatro PNG del selector desde
   `assets/selector/` usando `tools/materialize_selector_theme.py`;
4. solo entonces sube el artifact de Pages.

La página no contiene APKs ni datasets. Su Content Security Policy usa
`connect-src 'none'`: la selección y extracción del APK ocurre en el navegador
del usuario y el ZIP final se crea localmente.
