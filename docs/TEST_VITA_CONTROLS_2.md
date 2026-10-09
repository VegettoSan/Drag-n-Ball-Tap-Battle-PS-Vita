# Vita Controls Test 2 — ajustes después de la prueba en consola

Rama: `test/vita-controls`. `main` y v1.1 permanecen intactos. El usuario confirmó
Test 1 con controles Vita y pads ocultos; esta nueva versión requiere probar
los tres ajustes siguientes en consola.

## Instalar

Instalar **DBTapBattle-Vita-01.03-Controls-Test-2.vpk** con VitaShell. Usa la misma
burbuja **DB Tap Battle Controls Test**, `TITLE_ID=DBTBCT001`, versión `01.03`;
actualiza Test 1. Conserva su progreso en
`ux0:data/DBTapBattle/profiles/<Perfil>/save-controls-test.bin` y deja intacto el
`save.bin` estable. Los perfiles y sus recursos existentes siguen sirviendo;
no hace falta reinstalar sus datos.

## Selector

| Opción | Comportamiento |
|---|---|
| Solo táctil | Entrada táctil y configuración original del juego |
| Controles PS Vita | Botones Vita sobre los pads originales, con sus sprites ocultos |

Se pregunta cada vez que se abre un perfil. Se recuerda la opción resaltada por
perfil en `vita-controls.cfg`. Una preferencia antigua de Vita con pads visibles
resalta ahora Controles PS Vita y se guarda como Vita con pads ocultos al
confirmar. La pantalla frontal sigue disponible, también en modo Vita.

## Botones

| Vita | Acción |
|---|---|
| Cruceta / stick izquierdo | Ocho direcciones del stick original en combate |
| X | Ataque en combate; confirmar personaje en selección |
| Cuadrado | Primer atajo especial, pad 2 |
| Triángulo | Segundo atajo especial, pad 3 |
| Círculo | Tercer atajo especial, pad 4 |
| **L** | **Ira / acción especial del pad 6**, cuando el juego la permite |
| **R** | **Cuarto atajo especial, pad 5** |
| Start | Pausa original en combate; opciones de pausa por pantalla |
| Cruceta izquierda/derecha, selección de personajes | Personaje anterior/siguiente |
| Pantalla frontal | Menús y entrada original disponibles |

X pulsa el botón original de confirmación cuando la selección está preparada;
no cambia directamente índices ni tareas del motor. Una pulsación confirma una
vez: mantener X no selecciona automáticamente al personaje del siguiente paso
ni activa ataque al cambiar de escena. Soltar y pulsar de nuevo para confirmar
otra selección. En la pantalla de información de personajes no añade una
confirmación. El resto de los menús del juego sigue usando la pantalla táctil.

## Prueba solicitada

1. Abrir un perfil y comprobar que hay solo dos opciones. Elegir Controles
   PS Vita; comprobar que no aparecen los pads en combate.
2. Cambiar personaje con izquierda/derecha y seleccionarlo con X. Mantener X
   durante la transición y comprobar que no confirma el siguiente paso; soltar
   y volver a pulsar para otra selección cuando corresponda.
3. En combate, comprobar L para ira y R para el cuarto atajo especial, con el
   personaje y los requisitos originales necesarios para cada acción.
4. Cerrar/reabrir el perfil: debe volver a preguntar con Vita resaltado. Elegir
   Solo táctil y comprobar entrada/configuración original y progreso conservado.

## Alcance técnico

Cambios únicamente en adaptadores Vita y selector. Se conserva el pipeline de
compatibilidad de `main`; no se añaden parches a Controller, KeyData ni Game3.
La ocultación usa la vista temporal de imágenes DAC `-1` ya probada en Test 1;
no escribe PACs ni modifica sprites de otros elementos. El código de ocultación
no cambia en Test 2. Los nueve corpus DAC/CNV tienen evidencia host heredada,
no una aprobación de juego de nueve mods en consola. Un esquema desconocido
o ambiguo conserva los pads visibles al rechazar la ocultación de forma atómica.

Se mantienen los límites de Test 1: cinco contactos originales con prioridad
para dedos reales, tutorial táctil/de gestos y recarga de perfil si se cambia a
gestos desde ajustes durante la sesión Vita. No se introduce turbo ni una
adaptación general de menús.

Pruebas de entrada JVM, preferencias nativas y regresión Python aprobadas.
Compilación local de motor completo, sin workflow; la ejecución de Test 2 en
PS Vita queda pendiente del usuario. [Evidencia de compilación](evidence/vita_controls_test_2.json).
