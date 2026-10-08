# Investigación: ocultar los gráficos del pad desde Vita

> Implementation update: the experiment is now built on `test/vita-controls`.
> See [Controls Test 1](TEST_VITA_CONTROLS.md) and
> [local build evidence](evidence/vita_controls_test_1.json). The research below
> retains its original scope; hardware acceptance is still pending.

Fecha: 2026-10-08. Complementa la
[investigación de controles y selector](VITA_CONTROLS_RESEARCH_2026-10-08.md).

**Resultado:** hay una ruta viable sin modificar los métodos del motor:
un **overlay de recursos en memoria, aplicado por el adaptador Vita**, que
marca como ausentes las imágenes de las animaciones del pad. El dibujado
original ya omite esos objetos. Las zonas del `Controller` continúan activas,
por lo que pueden recibir tanto contactos físicos como los contactos
sintéticos propuestos para los botones Vita.

**Alcance probado:** inspección del DEX original y auditoría estructural de los
nueve APKs suministrados. En los nueve, el overlay hipotético afecta 25 campos
de imagen de 22 animaciones: **50 bytes**, sin cambiar el tamaño del DAC ni
bytes ajenos a esos campos. No se ha integrado al runtime, ejecutado el motor
con ese overlay ni probado la invisibilidad en una Vita. El VPK sigue igual.

## 1. Separación entre entrada y gráficos

`Game8`, caso 37, crea el stick y los botones mediante `Controller.AddPad`.
El caso 38 consulta sus estados y crea paneles con `CreatePanelSingle`,
`acttype=5`. Esos paneles llegan a las tareas 805/806 de `Game4`:
`_SetAct` y `_ActReq` ejecutan las animaciones del recurso activo del slot 5.
En combate normal ese recurso es `effect.pac`.

El motor no obtiene las zonas de entrada a partir de los PNG ni del tamaño
visible del panel. Ocultar su dibujo no necesita borrar pads, cancelar tareas,
alterar `Controller.GetKey` ni sustituir el procesamiento de combate.

La cadena gráfica relevante es:

1. El DAC selecciona el campo de imagen de cada fotograma; `_ActReqMain` lo
   copia a `ObjReq.img` como entero de 16 bits con signo.
2. `DrawExec` comprueba `r.img == -1` antes de llamar al dibujado de imagen.
   La comparación y rama se contrastaron con las instrucciones DEX del original.
3. Un campo `FF FF` produce `img=-1`: el panel conserva su tarea, tiempo,
   enlaces y posición, pero ese objeto no entrega su imagen a `Graphics2D`.

Esto es más preciso que aplicar transparencia a una textura completa. Tampoco
depende de capturar los pads después de que hayan quedado dibujados.

## 2. Qué gráficos pertenecen al pad

Los números son **roles de animación del motor original**, no offsets de
archivo, IDs OpenGL, coordenadas de atlas ni nombres cifrados del APK.

| Elemento | Animaciones raíz creadas por `Game8`, caso 38 |
|---|---|
| Stick neutro y ocho direcciones | 200–208 |
| Ataque en reposo/pulsado | 209, 210 |
| Comando 1 en reposo/pulsado | 330, 331 |
| Comando 2 en reposo/pulsado | 333, 334 |
| Comando 3 en reposo/pulsado | 336, 337 |
| Comando 8 en reposo/pulsado | 339, 340 |

**No basta ocultar estas 19 raíces.** El DAC contiene enlaces a otras capas:
el stick enlaza sus imágenes de base/dirección; las pulsaciones enlazan la
animación 211; las dos variantes del comando 8 enlazan 341/342. Estas últimas
tienen dos fotogramas cada una. La clausura del conjunto es de **22 acciones y
25 campos de imagen**. El análisis recorre enlaces con un conjunto de visitados:
hay referencias cíclicas entre 200 y 201 y no deben provocar recursión infinita.

Las acciones 332/335/338 comparten imágenes de atlas con el pad, pero no
pertenecen a la clausura observada. Se conservan. Esta distinción demuestra por
qué borrar píxeles o filtrar solo por UV puede ocultar otros estados.

El botón de ira utiliza una zona de entrada en el indicador superior. No se
incluye automáticamente su medidor en el overlay: vida, energía, ira, sincronía,
temporizador, información de combate y botón/menú de pausa deben seguir visibles.
El objetivo inicial es retirar el stick y los cinco botones visibles de la zona
inferior/lateral. Ampliarlo exige identificar por separado los gráficos afectados.

## 3. Compatibilidad comprobada con los datos de mods

Se utilizó el lector existente `community14.py` para variantes conocidas y
`web/private-dex.mjs` para descubrir las claves/alias PRIVATE de DBS Mobile y
DBZ Mobile. Se inspeccionó únicamente el recurso de efectos con contenido;
los stubs vacíos `res/raw` de APKs con datos en `assets` no se tomaron como fuente.

| APK suministrado | Recurso examinado | Resultado estructural |
|---|---|---|
| Original | `res/raw/effect.pac` | 22 acciones; 25 campos; 50 bytes |
| Android 14 | `assets/9B28.pac` | Igual alcance |
| Gen | `assets/effect.pac` | Igual alcance |
| Español Android 14 | `assets/B248.pac` | Igual alcance |
| Invasion Beta 3 | `assets/1E1C.pac` | Igual alcance |
| Zuper Samu | `assets/effect.pac` | Igual alcance |
| DBFZ v22 | `assets/7D98.pac` | Igual alcance; DAC/CNV diferentes |
| DBS Mobile v1 | `assets/49B9.pac` | Igual alcance; DAC diferente |
| DBZ Mobile v9 | `assets/54F1.pac` | Igual alcance; DAC diferente |

En el corpus, ningún registro ajeno comparte los bytes de imagen que se
ocultarían, ni enlaza a las 22 acciones. Los campos caben en sus propios
registros. Se verifican todos los fotogramas declarados, no solo el primero.
La imagen resuelta por cada campo también cabe en la tabla CNV del perfil.

Los DAC de DBFZ/DBS/DBZ y sus CNV no son todos iguales al original. Por ello,
**no copiar un `effect.pac` del original, ni usar 25 offsets fijos**. Resolver
las acciones, enlaces y campos en el DAC del perfil seleccionado mantiene las
texturas y modificaciones restantes del propio mod. Que compartan el esquema
no demuestra compatibilidad con cualquier mod futuro o cambio de código Android.

Evidencia reproducible, exclusivamente metadatos:
[vita_pad_visibility_2026-10-08.json](evidence/vita_pad_visibility_2026-10-08.json).

```sh
python tools/audit_pad_visibility.py /private/apks/*.apk \
  --output /private/pad-visibility.json
```

Requiere Python 3 y Node para reutilizar el lector PRIVATE del repositorio.
No escribe APKs, PACs, imágenes ni decompilaciones. El DAC modificado solo se
simula en memoria para contar cambios y obtener su hash.

## 4. Diseño recomendado del adaptador

Implementar la transformación en la frontera de recursos Vita, después de
resolver el perfil y normalizar la **tabla del contenedor protegido**, antes de
entregar el DAC al `GameData` original. El DAC de animaciones no es la tabla
BIN de ocho bytes: conserva su cabecera `0x1100`, directorio de acciones y
registros variables. No aplicar el decoder `binCnv` a este DAC por su extensión.

La modificación propuesta es de datos visuales servidos por la plataforma;
**no cambia bytecode Java, C generado ni métodos del motor**. El archivo de
disco y los PNG/RGBA/CNV quedan intactos. El overlay mantiene el tamaño y
estructura del contenedor: no necesita repack, sustitución de assets ni una
versión especial del mod para Vita.

Secuencia para `Controles Vita` + `Ocultar pad`:

1. Resolver el recurso de efectos de ese perfil con su codec vigente. Validar
   cabecera, rangos, directorio, registros, enlaces y fotogramas.
2. Resolver las 19 raíces y la clausura de sus enlaces; verificar que los campos
   a cambiar no pertenecen a registros ajenos ni son destinos de enlaces ajenos.
3. Preparar una lista acotada de offsets de campos de imagen. Servir `FF FF`
   exclusivamente en esas posiciones al leer el recurso en memoria.
4. Mantener intactos flags, tiempos, enlaces, rotación, escala, offsets, campos
   de movimiento/hit y todas las demás acciones.
5. Si el esquema/rol resulta ambiguo, conservar los gráficos y registrar el
   motivo. Debe poder usarse `Controles Vita` con pad visible aunque la ocultación
   no sea compatible con un perfil.

La auditoría actual comprueba propiedad estructural; el transformador de
producción necesita validar todos los rangos antes de publicar el overlay y
aplicarlo de forma atómica. No ocultar parcialmente una cadena inválida.

**Cachés y memoria:** el LRU actual usa ruta, filtro y metadatos del archivo;
no incluye la visibilidad. Añadir ese modo/generación a la identidad de un
resultado transformado, o mantener el recurso base inmutable y aplicar una
lista dispersa de sustituciones en el stream propio. No modificar un buffer
compartido que conserve otro lector. La opción dispersa evita duplicar un PAC
completo y debe respetar lecturas parciales, `skip`, límites y filtros que
excluyen DAC. Conservar el streaming y la transferencia de propiedad ya
validados en 00.23/00.33; 50 bytes de cambio no justifican una copia multi-MiB.

## 5. Selector antes de abrir el perfil

El selector es nativo; puede guardar esta elección antes de crear el motor.
Propuesta de opciones independientes:

| Opción de usuario | Entrada | Visibilidad |
|---|---|---|
| Táctil | Preferencia original de gestos/pad del perfil | Gráficos originales |
| Controles Vita, pad visible | Contactos sintéticos hacia el modo 1 original | Gráficos originales |
| Controles Vita, pad oculto | Misma entrada que la fila anterior | Overlay visual Vita |

Guardar entrada/visibilidad en la configuración lateral del perfil, sin
inventar valores 3/4 para `iControlType`. La preferencia de entrada original
sigue en `ConfigData[4]`; ocultar gráficos es exclusivamente una opción Vita.
Mantener disponible el panel frontal en ambos modos.

Aplicar la elección antes de la primera lectura de efectos evita recargar o
mutar objetos gráficos activos. La primera implementación debería cambiar de
modo al volver al selector/reiniciar el perfil. Si después se permite alternar
durante una partida, habrá que coordinar también los DAC ya cargados en Java:
invalidar solo el LRU nativo no restaura esos bytes.

El tutorial fuerza modo 2 temporalmente. Conservar sus gestos e indicadores
táctiles; el overlay se limita a las acciones del pad identificadas, sin ocultar
marcas genéricas de toque ni forzar modo 1 en cada frame.

Preferencia del usuario, 2026-10-08: mantener los menús táctiles. La única
adaptación adicional propuesta fuera del combate es D-pad izquierda/derecha
para cambiar de personaje en la selección. No hace falta un cursor Vita ni
adaptar todos los menús para la primera versión.

## 6. Alternativas revisadas

| Camino | Evaluación |
|---|---|
| Overlay DAC en el servicio de recursos Vita | Preferido: el dibujado original omite imágenes ausentes; roles y enlaces verificables por perfil |
| Filtrar índices en `VitaGles`/`dbtb_glDraw` | Posible frontera de plataforma, pero el batch puede mezclar objetos; exige atribución por primitiva y tratamiento de enlaces/estados |
| Ocultar una textura o poner alfa cero al atlas | Descartado: comparte recursos con otros efectos y estados |
| Poner alfa cero al objeto o al sprite sin revisar la ruta | Insuficiente: `DrawImage` no interpreta todos los ceros de alfa como invisibilidad; los pads observados usan DAC/CNV, no solo `DrawSprite` |
| Cancelar tareas del pad, borrar pads o modificar `DrawExec` | Fuera de la adaptación requerida; afecta tareas/entrada o modifica el motor |

La alternativa GL tendría que preservar orden, atributos y estado al omitir
solo los índices identificados. La semántica de primitivas indexadas se recoge
en la [especificación OpenGL ES 1.1 de Khronos](https://registry.khronos.org/OpenGL/specs/es/1.1/es_cm_spec_1.1.pdf).
La especificación no identifica los pads del juego ni certifica un filtro Vita.

## 7. Validación necesaria antes de publicar un VPK

- Prueba contra los métodos originales: visible/oculto, mismas tareas y entrada,
  retirada real del dibujo de todas las capas del pad.
- En Vita: direcciones, ataque y especiales sostenidos; combinaciones; toque
  frontal simultáneo; pausa, tutorial, menús, victoria y combates consecutivos.
- Repetir con efectos modificados y PRIVATE; comparar capturas para confirmar
  que no se pierden vida/energía/ira, efectos de lucha ni textos.
- Cambiar perfiles/modos y verificar invalidación de caché, lecturas fragmentadas,
  fallo de esquema sin ocultación parcial y archivos fuente sin cambios.

La investigación permite recomendar esta implementación. No establece aún que
el pad invisible esté disponible ni que los nueve mods hayan pasado esa prueba
gráfica en hardware.
