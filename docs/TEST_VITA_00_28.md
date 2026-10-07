# Prueba física PS Vita — 00.28 perfiles APK independientes

<!-- DBTB_DOC_STATUS:START -->
> **Project checkpoint:** 00.33 is the current hardware-confirmed development
> checkpoint for the tested paths. This file may document an earlier component
> or build; see [CURRENT_STATUS](CURRENT_STATUS.md) for authoritative status.
<!-- DBTB_DOC_STATUS:END -->


## Objetivo

Validar que un APK/dataset seleccionado funciona en Vita **sin instalar ningún
contenido en `ux0:data/DBTapBattle/game/`**.

00.28 elimina el fallback cruzado de recursos. Cuando se selecciona
`mods/<Profile>/`, PAC, tablas, audio y demás recursos se resuelven únicamente
desde ese perfil. Un archivo faltante ya no se toma silenciosamente de
`game/`.

Esto permite la instalación deseada:

```text
ux0:data/DBTapBattle/
├── game/                  # puede estar vacío
└── mods/
    ├── Android14/
    ├── Gen/
    ├── Invasion/
    └── ZuperSamu/
```

No es necesario instalar todos: **un único perfil bajo `mods/` debe bastar**.

## Identidad del VPK

- APP_VER: `00.28`
- TITLE_ID: `DBTB00001`
- Runtime source marker: `fa9d7b6`
- VPK: `DBTapBattle-Vita-00.28-Standalone-Profiles.vpk`
- bytes: `2,648,911`
- SHA-256:
  `4411302f1b7e673fe49c98bb9ce0b7fe47ed086a34e1ad03025735411d07cab2`
- eboot SHA-256:
  `5473e2fd7e1ea04cd9c207af61a440ef88d265b2338a093b9a616e1762b37f12`
- ELF SHA-256:
  `595052aec345e8b08f29cc650e0cfa085c2d37d9103769b778c73778808e9bf6`
- LiveArea: PASS.

00.24 continúa siendo el último checkpoint de gameplay físicamente confirmado.
00.28 es un candidato de prueba.

## Preparar la Vita sin Original

Para conservar una copia segura del estado anterior, renombrar temporalmente:

```text
ux0:data/DBTapBattle/game/
```

a algo que el runtime no use, por ejemplo:

```text
ux0:data/DBTapBattle/game_backup_00_28/
```

Al iniciar, el port puede volver a crear un directorio `game/` vacío. **No
copiar ningún PAC, OGG ni save desde el backup durante esta prueba.**

Instalar uno o varios APKs extraídos en:

```text
ux0:data/DBTapBattle/mods/<Profile>/
```

Si `game/common.pac` no existe, el selector:

- comienza resaltando el primer perfil instalado;
- muestra `ORIGINAL - DATA MISSING`;
- no permite iniciar ese Original vacío.

## Matriz host ya confirmada con los APK reales

Con `game/` vacío, el auditor 00.28 aceptó:

| Perfil | Personajes completos | Resultado host |
|---|---:|---|
| Gen | 13 | PASS |
| Android14 | 13 | PASS |
| Español Android14 | 13 | PASS |
| Invasion Beta 3 | 22 | PASS |
| Zuper/Samu | 92 | PASS |

Para Invasion se verificó además que `bobj00.pac` **no existe** y que el VFS
responde `missing selected profile resource: bobj00.pac`; no intenta tomarlo
de `game/`. El gate sigue aceptando el APK porque esa ausencia es válida en el
perfil auditado.

Esto confirma VFS/gate/formato, no reemplaza la prueba de gameplay físico.

## Prueba recomendada

### ZuperSamu

Probar selección y pelea con:

```text
13 20 35 42 54 79 91
```

Confirmar:

- roster completo visible/navegable;
- voces correctas;
- al menos un BGM MP3;
- al menos un BGM AAC/M4A;
- un BGM Vorbis;
- volver al menú y empezar otra pelea.

### Invasion

Probar:

```text
13 15 20 21
```

El 15 cubre el PAC grande de 101 entries y RGBA 736×500.

Para audio, si el flujo lo permite:

```text
MP3:     bgm_03 / bgm_06 / bgm_07
AAC/M4A: bgm_04 / bgm_05
Vorbis:  cualquier BGM baseline
```

### Android14 / Español / Gen

Si están instalados, abrir cada perfil **sin restaurar `game/`** y comprobar:

1. título/menu;
2. textos;
3. selección de personajes;
4. una pelea;
5. voces/SE/BGM;
6. regreso al menú.

## Qué significa un archivo faltante ahora

Si aparece:

```text
missing selected profile resource: X
```

**no copiar X desde Original para “arreglarlo”.**

Ese mensaje demuestra que el core original TeaVM está pidiendo algo que el APK
seleccionado no necesita o resuelve de otra manera. Hay que comparar esa ruta con
el `classes.dex`/contrato del APK y adaptar el port específicamente, conservando
el perfil autónomo.

Esto es especialmente importante para los APK protegidos que omiten
`bobj00.pac` y `font00.pac`.

## Qué enviar

Por cada perfil probado, idealmente una sesión separada. Enviar:

- `ux0:data/DBTapBattle/logs/runtime.log`;
- `psp2core-*.psp2dmp` si hay crash;
- nombre del perfil;
- personaje y pantalla donde ocurrió;
- video corto si el fallo es audible/visual.

No restaurar `game/` antes de copiar el log de un fallo de independencia.

## Criterio de aceptación

Un perfil se considera **standalone hardware-confirmed** cuando, con
`game/` vacío:

1. inicia desde el selector;
2. llega al menú;
3. selecciona personajes;
4. inicia y termina una pelea;
5. reproduce sus propios textos/voces/SE/BGM;
6. guarda en `mods/<Profile>/save.bin`;
7. no registra ninguna lectura desde `game/`.

Evidencia host/build:
[evidence/vita_standalone_profiles_00.28.json](evidence/vita_standalone_profiles_00.28.json).
