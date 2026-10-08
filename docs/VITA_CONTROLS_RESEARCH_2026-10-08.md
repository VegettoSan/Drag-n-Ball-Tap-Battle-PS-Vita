# Investigación: controles físicos Vita y elección antes de abrir un perfil

Fecha: 2026-10-08. Repositorio revisado: `b295b29e7f276cac75471d80ac7d41ede6160431`.

**Conclusión:** la ruta preferida es adaptar los botones y sticks Vita a los
contactos del **pad táctil original, modo 1**. El motor ya transforma esos
contactos en direcciones, ataques y comandos. La elección se puede añadir al
selector nativo y aplicar a la configuración del perfil antes de que el motor
la lea. No se necesita sustituir combate, IA, tareas ni `Controller`.

**Estado:** investigación estática y pruebas JVM realizadas; adaptación al VPK
pendiente. Este trabajo no cambia el ejecutable ni convierte la propuesta en
una función disponible. El punto de partida vigente es el candidato v1.1
VisualQuality aceptado en Vita, conservando v1.0 como referencia histórica.

## 1. Qué aportaba la investigación anterior

Se revisaron `ENGINE_MAP`, `PLATFORM_SERVICES`, `DECISIONS`, `PORTING_PLAN`,
`CURRENT_STATUS`, `CURRENT_RUNTIME_CONTRACT`, `PROJECT_RULES` y el código real
de entrada/selector/adaptadores.

- `tools/aot/InputProbe.java` ya probó `KeyData`/`Controller` originales:
  Begin/Move/End, identidad de punteros, diez slots y un botón **tipo 5**.
  El registro histórico contiene igualdad de salida JVM/TeaVM C.
- Esa prueba no era un mapeo de combate. El pad de combate encontrado ahora
  usa **tipo 1 para movimiento y tipo 4 para botones**.
- `src/input.cpp` lee Vita con `sceCtrlPeekBufferPositive` y `sceTouchPeek`.
  Normaliza contactos reales a IDs estables 0..4.
- `native/vita_platform.cpp::dbtb_frame` sigue fijando `events[40]` y
  `events[41]` a cero. Los botones funcionan en el selector, pero no controlan
  combate ni pausa en el bucle actual.
- `VitaEngine.java` convierte los contactos a coordenadas originales y llama
  `gw.keyData.Set/Clear` antes de `engine.Run`. Es la frontera adecuada para
  entregar también contactos sintéticos.
- Circle/Triangle causaron salidas de la aplicación al enviarse como Android
  Back. La neutralidad actual es intencional; no debe retirarse globalmente.

La investigación previa **sí sirve como base**, pero no certificaba el pad de
combate ni justificaba anunciar controles físicos terminados.

## 2. Evidencia recuperada del APK original

APK original SHA-256:
`b84f98a3ed70957354f358b7930bd8fb651cc89b74e16f8774ebd989fbf0899b`.

Se inspeccionó el DEX con Androguard 4.1.4 y JADX 1.5.6. La decompilación
sirve para navegación; constantes, firmas y ramas críticas se contrastaron
con instrucciones DEX. JADX presenta artefactos en otros métodos grandes:
su salida no debe recompilarse como reemplazo del motor.

| Hallazgo | Referencia original | Consecuencia para Vita |
|---|---|---|
| Preferencia de entrada | `ConfigData[4]`; array de 12906 bytes | Es una opción normal del juego, aislada del progreso restante |
| Modo 2 | `Game11`, casos 854/855 | Gestos/taps originales |
| Modo 1 | `Game11`, caso 855; `Game8`, caso 37 | Pad virtual con movimiento, ataque y cuatro acciones adicionales |
| Modo 0 | Rama de `Game8`, caso 37 | Pad reducido: movimiento y ataque; la pantalla de ajustes revisada ofrece 1/2 |
| Creación del pad | `CreateGamepad` → tarea 37 → `Game8` | No recrear tareas ni llamar a esta creación desde un segundo bucle Vita |
| Estado del pad | `Controller.GetKey(pad,0/1/2)` | Pulsación/liberación/mantenido, respectivamente |
| Consumo de entrada | `TCBManajer.Run` → `Controller.SetKey(gw.keyData)` | Entrar por `KeyData` conserva el procesamiento original |
| Comandos | `GetGamePadKey`, `CmdCheck`, `CommonNormal` | El motor conserva sus restricciones de energía, postura, orientación y cancelación |
| Tutorial | `Game1`, caso 685 | Fuerza temporalmente modo 2; no sobrescribirlo cada frame |
| Restauración | `Game1`, caso 690; `Game12`, casos 260/261 | El modo se vuelve a cargar; una asignación temprana aislada se puede perder |

### Pad de combate: posiciones y máscaras

Las coordenadas siguientes son **lógicas del motor**, no píxeles Vita ni
coordenadas del panel físico. Se obtienen de `Game8`, caso 37.

| Índice lógico `padID` | Posición | Tipo | Rango efectivo | Máscara | Uso verificado |
|---|---|---|---|---|---|
| 0 | `(90,237)` | 1 | 115 por eje | direcciones 1/2/4/8 | Movimiento |
| 1 | `(430,268)` | 4 | 24 por eje | `0x00004100` | Ataque; el intérprete decide su variante |
| 2 | `(340,268)` | 4 | 24 por eje | `0x00100000` | Atajo de comando 1, sujeto a `CmdCheck` |
| 3 | `(380,218)` | 4 | 24 por eje | `0x00200000` | Atajo de comando 2, sujeto a `CmdCheck` |
| 4 | `(440,188)` | 4 | 24 por eje | `0x00400000` | Atajo de comando 3, sujeto a `CmdCheck` |
| 5 | `(440,128)` | 4 | 24 por eje | `0x00800000` | Atajo de comando 8, sujeto a `CmdCheck` |
| 6 | `(60,40)` para jugador 0; `(420,40)` para jugador 1 | 4 | 54 por eje | `0x21000000` | Acción de ira/estado especial; el motor filtra disponibilidad |

`AddPad` divide entre dos los rangos declarados de tipos 1/4:
230→115, 48→24 y 108→54. Tipo 4 comprueba límites rectangulares estrictos,
aunque el gráfico parezca circular. No tratar 48 como radio.

El stick tipo 1 resta **10 a X e Y del contacto** antes de calcular el ángulo.
Por eso su punto neutro matemático es `(100,247)`, no `(90,237)`.
Puntos comprobados con desplazamiento 64:

| Dirección | Contacto lógico | Máscara mantenida |
|---|---|---|
| Arriba | `(100,183)` | 1 |
| Arriba/derecha | `(164,183)` | 9 |
| Derecha | `(164,247)` | 8 |
| Abajo/derecha | `(164,311)` | 10 |
| Abajo | `(100,311)` | 2 |
| Abajo/izquierda | `(36,311)` | 6 |
| Izquierda | `(36,247)` | 4 |
| Arriba/izquierda | `(36,183)` | 5 |

Para movimiento analógico se puede limitar el desplazamiento a un radio
seguro de 64, con zona muerta Vita e histéresis. Su ajuste es una decisión
pendiente de comodidad en consola. No añadir turbo ni repetir Begin cada
frame. El tipo 1 actualiza la dirección mantenida al mover un contacto sin
generar otra pulsación inicial: esa semántica fue comprobada.

### Escala actual

`VitaEngine` usa `s=320/544`, ancho lógico truncado 564 y offset X=42:

`logicalX = trunc(screenX * s) - 42`

`logicalY = trunc(screenY * s) - gw.iScreenOffsetY`

En el arranque inspeccionado el offset Y es cero. Si se generan eventos en
píxeles, invertir la escala con cuidado y verificar el resultado después del
truncado. La alternativa preferida es que un adaptador Java escrito para Vita
genere directamente los contactos lógicos después de convertir el toque real.
El APK original no se modifica en ninguno de estos casos.

## 3. Ruta recomendada y alternativas

| Ruta | Evaluación |
|---|---|
| Botones Vita → contactos del pad modo 1 → `KeyData` → motor original | **Preferida.** Tiene contrato recuperado y pruebas del `Controller` original; conserva timing, combinaciones e interpretación |
| Botones Vita → taps/swipes sobre luchadores en modo 2 | Útil para compatibilidad/tutorial, pero como ruta principal exige seguir posición, cámara y contexto; tiene más condiciones |
| Escribir `Joy`, `JoyTrig`, `PlayerKey` o `TouchesCommand` directamente | Evitar: hay buffers, filtros y etapas originales que una escritura externa puede saltarse o perder |
| `SetKeyCode` como supuesto mando físico | Incorrecto: configura códigos de un pad; no inyecta el estado de un botón físico |
| Parchear `Controller`, `Game8`, `CmdCheck` o tareas de combate | No necesario para la ruta propuesta; fuera del alcance |
| Bluetooth como mando | No corresponde: el subsistema del APK es transporte de datos de batalla, no un driver de gamepad |

El adaptador Vita debe leer los controles una vez por frame, calcular
pulsar/mantener/soltar, producir contactos estables y dejarlos al `Run`
original. No agregar un segundo `Controller.SetKey`, pues el motor ya lo llama
y hacerlo dos veces alteraría los bordes y contadores.

La documentación primaria VitaSDK confirma polling de controles mediante
[`sceCtrlPeekBufferPositive`](https://docs.vitasdk.org/psp2_2ctrl_8h.html) y
toques mediante [`sceTouchPeek`](https://docs.vitasdk.org/group__SceTouchUser.html).
El port ya usa ambas APIs; no se requiere un plugin de remapeo del sistema.

### Distribución inicial propuesta — aún no probada en Vita

| Control Vita | Entrada original propuesta |
|---|---|
| Cruceta / stick izquierdo | Stick virtual, ocho direcciones |
| X | Pad 1: ataque |
| Cuadrado | Pad 2: primer atajo especial |
| Triángulo | Pad 3: segundo atajo especial |
| Círculo | Pad 4: tercer atajo especial |
| L | Pad 5: cuarto atajo especial |
| R | Pad 6: ira/acción especial habilitada por el motor |
| Start | Pausa original, solamente con validación de contexto |
| Stick derecho + X fuera de combate | Cursor Vita para tap/arrastre en menús y tutorial; propuesta de compatibilidad |
| Pantalla frontal | Disponible en ambos modos mediante un único asignador de contactos |

Los nombres finales de las técnicas dependen del personaje/datos. No prometer
que un atajo ejecuta una técnica sin energía o fuera de su estado permitido.
La defensa, saltos y movimientos deben conservar la interpretación direccional
del juego; no crear nuevas acciones de combate para hacer el mapeo.

### Menús, tutorial y pausa

- La firma del pad de combate debe comprobarse leyendo el `Controller`/`padID`
  actual desde el adaptador Vita del mismo paquete. No identificar combate
  únicamente por que `iControlType==1`: muchos menús usan otros pads.
- Fuera del pad válido, pasar a cursor/tap/arrastre y suprimir las entradas de
  ataque. Priorizar el toque real conserva compatibilidad con pantallas no
  exploradas. Menús totalmente navegables con botones requieren comprobarlos;
  esta investigación no certifica navegación directa de todas sus opciones.
- El tutorial establece modo 2 en caso 685. Respetarlo. Un cursor con arrastre
  puede representar sus gestos, pero su comodidad y cobertura están pendientes.
  No forzar modo 1 cada frame ni omitir el tutorial.
- Start no debe convertirse globalmente en Android Back. La rama original
  `Game1`, caso 676, pausa cuando `iBackKeyType==1` y no hay loading. En modo
  Bluetooth 8 la misma entrada puede desconectar/salir: excluirlo de este
  mapeo hasta auditarlo. Comprobar además tarea/escena, pausa y transición.
- El cursor en menús debe ocupar ID lógico 0 cuando esté solo: numerosas
  pantallas consultan explícitamente `TouchesStatus[0]`. No darle ID 7/9.

## 4. Elección en el selector antes del perfil: viable

`runBootSelector` pertenece enteramente a Vita. `BootChoice` hoy sólo contiene
`profile_directory`; se puede extender con una elección de entrada sin cambiar
la pantalla original del APK.

UX propuesta: seleccionar perfil → **Pantalla táctil / Controles Vita** → abrir
perfil. Añadir una página pequeña con el mismo fondo/botones mantiene legibles
las seis filas actuales. Recordar la preferencia por perfil en un archivo Vita
pequeño, por ejemplo `controls_vita.cfg`, no en PACs ni en el extractor.

**Pantalla táctil** puede conservar la preferencia de gestos/pad que el usuario
tenía en los ajustes originales. **Controles Vita** requiere pad modo 1.
Guardar también la preferencia táctil anterior permite restaurarla al cambiar
de modo. Si no existe configuración Vita, mantener el comportamiento actual.

### Momento preciso y guardado

1. El selector recoge perfil y modo. Todavía no hay motor original activo.
2. `dbtb_initResources` selecciona el perfil, carga/crea su `save.bin` de
   **12906 bytes**, sincroniza roster y mantiene `save_cache`.
3. Aplicar la opción de controles por el servicio de guardado Vita antes de
   devolver éxito de `dbtb_start`. En modo Vita, el byte 4 debe valer 1.
4. El motor hace `Init` y después su tarea inicial 682 llama `FILEInit` →
   `_FILELoad`. Ésta copia los bytes del servicio a `ConfigData`.
5. Los casos 690/261 recuperan `iControlType[0]` de `ConfigData[4]`. El flujo
   original sigue siendo responsable de crear el pad y restaurar su modo.

**Diseño preferido:** tratarlo como la configuración normal elegida por el
usuario. Utilizar `dbtb_writeSave` con un byte, posición 4 y `truncate=0`,
después de validar el tamaño; el servicio existente publica con temporal,
`fsync`, cierre y `rename`, y actualiza la caché. Añadir backup de la primera
transición de opción y guardar/restaurar la preferencia táctil en el sidecar.
Comparar los otros 12905 bytes antes/después en pruebas. No regenerar ni
reemplazar el guardado por el seed, ni tocar monedas/cartas/desbloqueos.

**Detalle importante:** editar sólo el archivo en disco después de
`dbtb_initResources` no basta: el motor lee `save_cache`. La mutación tiene que
pasar por el servicio que actualiza ambos. Asignar únicamente
`iControlType[0]=1` antes de `engine.Init` tampoco resiste el `_FILELoad` posterior.

Una alternativa de override sólo en la copia que entrega `readSave` evita una
escritura inicial, pero los guardados originales posteriores pueden persistir
esa preferencia. Garantizar que nunca cambia el byte persistente necesitaría
filtrar escrituras completas/parciales y reconciliar ajustes. Es más complejo
que aplicar una opción explícitamente elegida; no es la primera recomendación.

Si la configuración o el guardado son inválidos, no inventar uno nuevo ni
continuar anunciando que el modo fue aplicado. Mantener la recuperación actual
y mostrar el fallo de forma concreta. Definir precedencia si el usuario cambia
gestos/pad desde los ajustes mientras utiliza Controles Vita: actualizar la
preferencia Vita y el adaptador de forma coordinada, o informar que debe volver
al selector. No luchar contra el cambio reescribiendo el motor cada frame.

## 5. Multitouch y cambios de escena

`KeyData` acepta diez slots, pero el bucle original expone contactos 0..4 a
sus arreglos `Touches*`. Las cinco entradas activas son un límite compartido
entre dedos reales y contactos sintéticos, no cinco adicionales por origen.

- Un único asignador debe gestionar ambos orígenes. Mantener identidad mientras
  un contacto esté activo y emitir End/Clear al liberar.
- No reutilizar un ID para otra acción antes de liberar al dueño anterior.
- Conservar todos los End aun si se alcanza el límite. El bridge puede entregar
  diez eventos por frame, aunque sólo cinco contactos queden activos.
- Movimiento + seis botones supera cinco. Documentar prioridades para nuevas
  entradas al saturarse; no expulsar silenciosamente una entrada sostenida.
  Validar especialmente dedos reales mezclados con botones y combinaciones.
- Al entrar/salir de selector, carga, pausa, resultados o suspensión, liberar
  sólo contactos del adaptador y bloquear Begin de controles ya sostenidos
  hasta su liberación. No modificar estado de combate para limpiar la entrada.
- No usar `Controller.ClearKey` como limpieza genérica: su implementación
  limpia también geometría/configuración de pads. Retirar contactos `KeyData`;
  permitir que el motor haga sus propios Init/recreación.
- No agregar lecturas de archivos, asignaciones grandes o logs por frame.
  Preferencia y bindings se cargan al entrar al perfil; los eventos usan buffers
  pequeños reutilizables.

## 6. Corpus de nueve APKs

Inventario reproducible en
[`evidence/vita_controls_apk_inventory_2026-10-08.json`](evidence/vita_controls_apk_inventory_2026-10-08.json).
Contiene SHA-256 completos, identificadores, contadores y huellas de
instrucciones resueltas; no incluye código del juego.

| APK | Familia de entrada inspeccionada | Resultado estático |
|---|---|---|
| DBTapBattle | `Controller` / `KeyData` con nombres | Fuente del contrato y de pruebas JVM |
| gen | Misma familia | Huellas de todos los métodos de entrada iguales al original |
| DragonBallZuperSamuGamerYT | Misma familia | Huellas de todos los métodos de entrada iguales al original |
| tap battle Android 14 | `ext/i` / `ext/r` | Referencia de la familia ofuscada |
| Español Android 14 | Misma familia ofuscada | Huellas de entrada y llamadores AddPad iguales a Android14 |
| Invasion Beta 3 | Misma familia ofuscada | Huellas de entrada y llamadores AddPad iguales a Android14 |
| Dbfz v22 | Misma familia ofuscada | Huellas de entrada y llamadores AddPad iguales a Android14 |
| DBS Mobile v1 | Misma familia ofuscada | Huellas de entrada y llamadores AddPad iguales a Android14 |
| DBZ Mobile v9 | Misma familia ofuscada | Huellas de entrada y llamadores AddPad iguales a Android14 |

Igualdad de estas huellas no prueba equivalencia de todo el APK. No se afirma
identidad bytecode entre las dos familias. El VPK actual ejecuta **el core
original fijado**, no el DEX particular de cada mod; por eso el primer adaptador
se dirige a un contrato original común, sin bindings por nombre de APK.
Cambios de sprites/tablas de un mod aún requieren prueba visual y de combate.

## 7. Pruebas realizadas y límites

Se convirtió privadamente el original con dex2jar 2.4. ECJ 3.37.0 compiló
`InputProbe` y el nuevo `VirtualPadProbe` contra ese JAR. Se ejecutaron en JVM.
El nuevo harness llama los tipos 1/4 del original; no sustituye su lógica.

| Prueba | Resultado |
|---|---|
| Repetición del InputProbe previo | PASS en JVM |
| Ocho direcciones, pulsación/mantenido/liberación | PASS |
| Seis botones, máscaras y liberación | PASS |
| Rango efectivo 115/24/54 | PASS |
| Centro de ira aislado del stick | PASS |
| Cinco IDs 0..4 simultáneos y retirada completa | PASS |
| Cambio de dirección con contacto sostenido | PASS; cambia mantenido, no genera otro Begin |
| VPK, polling Vita, nueva generación TeaVM, escena/pausa/selector | **No ejecutados en esta investigación** |

Durante la investigación se descartó una sospecha inicial de solapamiento del
centro de ira con el stick. El motivo era interpretar el rango declarado como
rango efectivo: el original lo divide entre dos. La prueba se corrigió para
comprobar el contrato real; no se añadió un workaround al runtime.

Evidencia de salida y alcance:
[`evidence/vita_controls_host_probe_2026-10-08.json`](evidence/vita_controls_host_probe_2026-10-08.json).

Reproducción, con insumos privados fuera de Git:

```sh
python -m pip install androguard==4.1.4
python tools/audit_controls_apks.py /private/apks/*.apk --output /private/controls.json
bash /private/dex-tools-v2.4/d2j-dex2jar.sh --force --output /private/original.jar /private/DBTapBattle.apk
java -jar /private/ecj-3.37.0.jar -8 -d /private/probe-classes -cp /private/original.jar tools/aot/InputProbe.java tools/aot/VirtualPadProbe.java
java -cp /private/probe-classes:/private/original.jar com.namcobandaigames.dragonballtap.apk.VirtualPadProbe
```

JAR, clases originales y decompilaciones se mantienen fuera del repositorio.
Las pruebas actuales no equivalen a la nueva comparación TeaVM ni a una prueba
de pelea; esos niveles deben completarse durante implementación.

## 8. Implementación futura por etapas

1. Añadir elección al selector/`BootChoice` y configuración Vita por perfil.
   Probar cancelación, no-data, persistencia y restauración; comprobar que sólo
   cambia el byte de opción permitido en el guardado, sin progreso perdido.
2. Exponer snapshot de botones/sticks desde el polling nativo y añadir un
   adaptador de contactos lógicos en `VitaEngine`, reutilizando `KeyData`.
   Comparar el probe ampliado en JVM y TeaVM antes de enlazar un VPK.
3. Implementar asignador compartido de cinco contactos, neutralización y
   detección del pad real. Probar saturación, multitouch mixto, soltar durante
   loading/pausa y controles sostenidos durante transiciones.
4. Probar un VPK experimental en Vita: original/Gen, movimientos y diagonales,
   ataque sostenido, especiales con/sin energía, cambios de lado y combates
   sucesivos. Comprobar Start sin salida accidental.
5. Probar menús, tutorial, práctica, invasión y mods protegidos; comparar audio,
   rendimiento, textos, guardados y suspensión con el runtime aceptado.
6. Publicar el soporte solamente después de confirmar el alcance en consola;
   mantener el VPK aceptado anterior para rollback.

Archivos previstos: `src/input.hpp/.cpp`, `src/ui.hpp/.cpp`,
`native/vita_platform.cpp`, bridge/`NativePlatform.java`, `VitaEngine.java`,
un adaptador Vita nuevo y configuración en el servicio de guardado.
`TCBManajer`, `Controller`, `KeyData`, `CmdCheck`, IA, PACs y codecs conservan
su implementación original. No hace falta actualizar extractores/APKs para
añadir botones físicos.

## Fuentes primarias y trazabilidad

- APKs del proyecto: identificados por hash en el inventario adjunto.
- Código público del port en el commit indicado al inicio; referencias actuales
  en [PLATFORM_SERVICES](PLATFORM_SERVICES.md) y [ENGINE_MAP](ENGINE_MAP.md).
- [VitaSDK Controller](https://docs.vitasdk.org/psp2_2ctrl_8h.html) y
  [VitaSDK Touch](https://docs.vitasdk.org/group__SceTouchUser.html).
- [dex2jar 2.4](https://github.com/pxb1988/dex2jar/releases/tag/v2.4),
  [JADX 1.5.6](https://github.com/skylot/jadx/releases/tag/v1.5.6).
- Como comparación de frontera plataforma se revisó el
  [wrapper de Prince of Persia de MetalSyntax](https://github.com/MetalSyntax/prince-of-persia-classic-psvita-port).
  Su sistema de remapeo ilustra la separación de entrada, pero carga bibliotecas
  nativas Android; sus hooks/bindings no son intercambiables con este port Java AOT.
