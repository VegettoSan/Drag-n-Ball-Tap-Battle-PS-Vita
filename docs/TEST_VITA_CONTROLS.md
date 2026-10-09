# Vita Controls Test 1 — rama experimental

**Registro histórico:** el usuario probó esta versión con controles Vita y pads
ocultos y reportó que funcionó perfectamente. No indicó perfil ni adjuntó logs.
El esquema y las tres opciones siguientes describen Test 1; la versión actual
es [Controls Test 2](TEST_VITA_CONTROLS_2.md), con L/R invertidos, X para confirmar
personaje y solo dos opciones de entrada.

Esta prueba adapta entrada y recursos en PS Vita. No añade parches al
motor: conserva `TCBManajer`, `Controller` y `KeyData` del pipeline existente.
Se mantienen las adaptaciones de compatibilidad ya presentes en main
(apertura de recursos, codificación de texto y capacidad del roster). No está aprobada aún en
hardware y no debe fusionarse a `main` hasta completar la aceptación.

## Instalación y elección

Instalar `DBTapBattle-Vita-01.02-Controls-Test-1.vpk` con VitaShell. Su burbuja
**DB Tap Battle Controls Test** usa `TITLE_ID=DBTBCT001`, distinto de la estable
`DBTB01178`. Usa los mismos perfiles ya instalados en
`ux0:data/DBTapBattle/profiles/<Perfil>/`; no requiere extraer de nuevo los mods.

Después de seleccionar un perfil, elegir:

| Opción | Comportamiento |
|---|---|
| TACTIL | Entrada y gráficos originales, con la preferencia táctil del guardado |
| VITA - PAD VISIBLE | Botones Vita sobre el pad original de modo 1; gráficos originales |
| VITA - PAD OCULTO | Mismos botones; el adaptador Vita omite las imágenes del pad |

La elección se recuerda por perfil en `vita-controls.cfg`. Cambiarla requiere
cerrar y volver a iniciar la aplicación. Los menús del juego siguen siendo
táctiles; el selector Vita admite cruceta/stick, X y pantalla táctil.

Cada perfil usa **save-controls-test.bin** para esta burbuja. En su primer
arranque se copia el `save.bin` estable, si existe y tiene el tamaño válido;
en otro caso se usa la semilla del VPK. Los avances de esta prueba quedan en
ese guardado independiente. La versión estable sigue usando `save.bin`.
No borrar ni sustituir el guardado estable para probar los controles.

## Botones

| Vita | Acción original |
|---|---|
| Cruceta / stick izquierdo | Ocho direcciones del stick virtual |
| X | Ataque, pad 1 |
| Cuadrado | Primer atajo especial, pad 2 |
| Triángulo | Segundo atajo especial, pad 3 |
| Círculo | Tercer atajo especial, pad 4 |
| L | Cuarto atajo especial, pad 5 |
| R | Ira/acción especial del pad 6, cuando el juego la permite |
| Start | Toque de pausa original durante combate; opciones de pausa por pantalla |
| Cruceta izquierda/derecha, selección de personajes | Personaje anterior/siguiente mediante las flechas originales |
| Pantalla frontal | Disponible en todos los modos |

Las técnicas dependen del personaje, energía y reglas originales. Mantener un
botón conserva la entrada sostenida del pad; no genera un turbo artificial.
Las flechas de personaje avanzan una vez por pulsación: soltar antes de otro
paso y esperar a que termine la animación de cambio. Select y stick derecho
no tienen una función añadida en esta prueba.

## Prueba en consola

1. Iniciar TACTIL, recorrer menús, elegir personaje y jugar un combate. Comparar
   con la burbuja estable: imagen, audio, progreso, menús y controles táctiles.
2. Reiniciar con VITA - PAD VISIBLE. Probar las ocho direcciones, cada botón
   separado y mantenido, soltar, cambiar dirección y combinar movimiento con
   ataque/especial/ira. Anotar personaje, técnica y resultado.
3. Probar Start mientras se mantiene movimiento/ataque. Debe abrir la pausa,
   sin salir de la aplicación. Reanudar por pantalla y volver a pulsar los
   botones después de soltarlos. Probar victoria y un segundo combate.
4. En selección, usar izquierda/derecha; tocar para confirmar. X/Círculo no
   deben navegar ni confirmar menús originales. Mantener X durante una
   transición no debe confirmar otra pantalla ni atacar al llegar al combate.
5. Repetir con VITA - PAD OCULTO y el mismo personaje/escenario. Comparar
   capturas: desaparecen stick/botones/capas del pad; siguen vida, energía,
   ira, efectos del combate, personajes y textos. Repetir con varios mods,
   incluidos DBFZ y PRIVATE DBS/DBZ.
6. Mezclar un dedo en pantalla con movimiento y botones. Probar cinco
   contactos y liberarlos; ningún contacto debe quedar atascado. Verificar
   tutorial por pantalla, cambios de perfil/modo, suspensión/reanudación y
   que el progreso de la burbuja estable permanezca igual.

## Límites deliberados

El original consume cinco IDs de contacto (0..4). Un dedo tiene prioridad
sobre el contacto simulado que ocupe su ID; entradas físicas que no quepan
esperan a que quede uno libre. Start necesita el ID 0 para la pausa original;
si lo ocupa un dedo, la solicitud espera a que se retire, dentro del combate.

Los botones se habilitan solo al reconocer el pad y tarea originales activos;
se liberan al salir de combate/selección y deben soltarse para volver a
activarlos. El tutorial conserva su cambio original a gestos. Si se cambia a
modo de gestos desde ajustes durante una sesión Vita, los botones se
neutralizan hasta volver a cargar el perfil; no se fuerza la configuración en
cada frame ni se cambia la lógica original de ajustes.

La ocultación valida DAC/CNV, enlaces y propiedad de registros del `effect.pac`
normalizado de cada perfil. Si un mod trae un esquema distinto o ambiguo,
conserva el pad visible y deja un diagnóstico, sin ocultación parcial. La
validación estructural de nueve APKs y las pruebas nativas no equivalen a
haber visto esos nueve mods funcionar en Vita.

Enviar para aceptar la prueba: perfil/personaje, modo elegido, combinaciones
probadas, resultado de pausa/transiciones y capturas visible/oculto. Si hay un
fallo, conservar `ux0:data/DBTapBattle/logs/runtime.log` y el coredump si existe.
