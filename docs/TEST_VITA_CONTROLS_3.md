# Vita Controls Test 3 — pausa, volver y texto

<!-- DBTB_DOC_STATUS:START -->
> **Repository status — 2026-10-09:** main contains prepared v1.2 (`01.02` / `DBTB01178`),
> retaining v1.1 and approved Vita controls; dialogues use touch. Exact v1.2 hardware
> retest is pending; published release is 1.1. [Current contract](CURRENT_RUNTIME_CONTRACT.md) · [Status](CURRENT_STATUS.md).
> Historical build sheet: its identities/results apply to the named build only.
<!-- DBTB_DOC_STATUS:END -->

Rama `test/vita-controls`; `main` permanece intacta. Test 2 fue aprobado por
el usuario. Esta versión añade tres atajos mediante toques originales y
requiere comprobarlos en PS Vita.

## Resultado posterior en Test 4

Test 4 conserva estos atajos sin cambios de Java. El usuario confirmó pausa,
pero Círculo fuera de pausa y X en diálogos fallaron. El alcance host de abajo
no constituye aceptación de esos dos atajos en consola. La corrección está en
[Test 5](TEST_VITA_CONTROLS_5.md); [reporte](evidence/vita_controls_hardware_report_test_4.json).

## Instalar

Instalar **DBTapBattle-Vita-01.04-Controls-Test-3.vpk** en VitaShell. Actualiza
la misma burbuja **DB Tap Battle Controls Test** (`DBTBCT001`, `01.04`) y
conserva `profiles/<Perfil>/save-controls-test.bin`. Los datos del perfil siguen
sirviendo y `save.bin` estable permanece intacto. No borrar la burbuja previa
para actualizar. El selector sigue preguntando cada apertura de perfil con
**Solo táctil** / **Controles PS Vita**, recordando la opción resaltada; Vita
oculta los pads táctiles. Los atajos siguientes pertenecen al modo Vita.

## Referencia de controles

| Botón | Acción |
|---|---|
| Cruceta / stick izquierdo | Moverse en combate, ocho direcciones |
| Cruceta izquierda / derecha | Cambiar personaje en selección |
| X | Ataque; confirmar personaje preparado; avanzar/revelar texto interactivo |
| Cuadrado | Atajo especial 1, pad 2 |
| Triángulo | Atajo especial 2, pad 3 |
| Círculo | Atajo especial 3 en combate; volver en menús con botón original visible |
| R | Atajo especial 4, pad 5 |
| L | Ira, pad 6, cuando el juego lo permite |
| Start | Pausar combate; reanudar desde el menú principal de pausa |
| Pantalla frontal | Entrada y demás opciones de menú originales |
| Select / stick derecho / botón PS | Sin atajo del juego; PS conserva su función del sistema |

Los especiales dependen del personaje y sus requisitos. Start en el menú
principal de pausa activa el volver original; en sus ajustes anidados, Círculo
regresa primero. Círculo necesita el panel de volver visible y su consumidor
original auditado; no contesta automáticamente diálogos de Sí/No. X en texto
produce el mismo toque que recibe el guion original: este decide si completar
la escritura o pasar al siguiente bloque. No modifica sus indicadores.
Mantener un botón no repite estos atajos: soltar antes de volver a pulsar.

Guía de referencia entregada: **Guia-Controles-PS-Vita-Tap-Battle.png**, 1920 ×
1320, azul/plata/dorado del selector. Ilustración decorativa generada con
ImageGen y leyenda SVG exacta, renderizada a PNG; no son capturas de hardware.
Su SHA-256 figura en [la evidencia](evidence/vita_controls_test_3.json).

## Prueba en consola

1. Elegir Controles PS Vita. Pausar con Start, soltar y volver a pulsarlo:
   debe reanudar una vez. Mantener Start durante entrada/salida de pausa no
   debe cerrarla inmediatamente ni abrirla otra vez.
2. Pausar y pulsar Círculo. Abrir ajustes desde pausa por táctil y usar Círculo
   para volver. Probar el botón visible de volver en selección y otros menús.
   Sus confirmaciones conservan las opciones táctiles originales.
3. En un diálogo como el de victoria mostrado, soltar y pulsar X para completar
   o avanzar el texto. Mantener X no debe saltarse varios bloques ni activar
   ataque al comenzar el siguiente combate. Soltar/repetir cuando corresponda.
4. Comprobar que un dedo real conserva prioridad y que Círculo/Start esperan
   si ocupa el contacto 0. Probar también Solo táctil, progreso conservado,
   selección con X, L para ira y R para el cuarto atajo.

## Evidencia y límites

JVM contra clases originales: combate/selección, CheckBack original, Start,
Círculo, X, prioridad táctil, cargas, tareas inactivas, botones mantenidos y
cambios de pantalla aprobados. Python: 41 aprobados, 20 omisiones existentes.
Generación fresca 468 clases/4103 métodos y compilación local de motor completo
aprobadas; sin workflow. Integridad ZIP/SFO/SELF/semilla/selector/LiveArea
aprobada. [Evidencia](evidence/vita_controls_test_3.json).

No se añaden parches al motor. El JAR procesado coincide en todas sus entradas
con el pipeline existente de main regenerado sobre el mismo JAR privado;
Controller y KeyData coinciden además con el original. No cambian la ocultación
DAC, preferencias ni rutas de guardado. La evidencia de nueve corpus DAC/CNV
es heredada, no nueve mods probados en consola. Los mods que conservan esas
pantallas originales usan los mismos atajos; nuevas interfaces de código
requieren auditar sus controles. Cinco contactos, prioridad de dedos reales,
tutorial táctil/de gestos y recarga tras cambiar a gestos siguen vigentes.
**Test 3 en hardware queda pendiente.** [Auditoría](VITA_MENU_SHORTCUTS_2026-10-09.md).
