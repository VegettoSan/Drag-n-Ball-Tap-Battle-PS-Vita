# Original renderer → vitaGL mapping

Source: supplied DEX, disassembled with androguard and decompiled locally with
jadx 1.5.6. API availability checked against vitaGL commit
`cdbba4232cb93a741ba190be9a32143dfed12d8d`. Availability is source-level evidence,
**not a GPU fidelity or hardware test**. The adapter is now implemented in the
full AOT engine: earlier Vita menu/selection/battle are confirmed, and 00.19
restores text. 00.21 starts/reaches the menu but fails character loading; 00.22 repairs its
mask contract. Physical selection recovery and exhaustive pixel/state parity
remain pending.
Current checkpoint: [CURRENT_STATUS](CURRENT_STATUS.md).

| Original API | Vita equivalent | Classification / adaptation |
|---|---|---|
| `glBindTexture` | `glBindTexture` | Directly compatible API; preserve exact original state and parameters. |
| `glBlendFunc` | `glBlendFunc` | Directly compatible API; preserve exact original state and parameters. |
| `glClear` | `glClear` | Directly compatible API; preserve exact original state and parameters. |
| `glClearColor` | `glClearColor` | Directly compatible API; preserve exact original state and parameters. |
| `glColor4f` | `glColor4f` | Directly compatible API; preserve exact original state and parameters. |
| `glColorPointer` | `glColorPointer` | Small adapter: direct native buffer pointers and lifetime; preserve GL_SHORT vertices, float color/UV, GL_UNSIGNED_BYTE indices. |
| `glDeleteTextures` | Native texture release | Translate Java IDs/offset; preserve shared immutable-cache ownership and delete uncached/evicted storage exactly once. |
| `glDisable` | `glDisable` | Review original enums: invalid legacy disables/hints must not poison error reporting. |
| `glDisableClientState` | `glDisableClientState` | Directly compatible API; preserve exact original state and parameters. |
| `glDrawElements` | `glDrawElements` | Small adapter: direct native buffer pointers and lifetime; preserve GL_SHORT vertices, float color/UV, GL_UNSIGNED_BYTE indices. |
| `glEnable` | `glEnable` | Directly compatible API; preserve exact original state and parameters. |
| `glEnableClientState` | `glEnableClientState` | Directly compatible API; preserve exact original state and parameters. |
| `glGenTextures` | `glGenTextures` | Small adapter: C pointer array (remove Java array offset). |
| `glGetString` | `glGetString` | Directly compatible API; preserve exact original state and parameters. |
| `glHint` | `glHint` | Review original enums: invalid legacy disables/hints must not poison error reporting. |
| `glLoadIdentity` | `glLoadIdentity` | Directly compatible API; preserve exact original state and parameters. |
| `glMatrixMode` | `glMatrixMode` | Directly compatible API; preserve exact original state and parameters. |
| `glOrthof` | `glOrthof` | Directly compatible API; preserve exact original state and parameters. |
| `glPopMatrix` | `glPopMatrix` | Directly compatible API; preserve exact original state and parameters. |
| `glPushMatrix` | `glPushMatrix` | Directly compatible API; preserve exact original state and parameters. |
| `glScalef` | `glScalef` | Directly compatible API; preserve exact original state and parameters. |
| `glShadeModel` | `glShadeModel` | Directly compatible API; preserve exact original state and parameters. |
| `glTexCoordPointer` | `glTexCoordPointer` | Small adapter: direct native buffer pointers and lifetime; preserve GL_SHORT vertices, float color/UV, GL_UNSIGNED_BYTE indices. |
| `glTexEnvf` | `glTexEnvf` | Directly compatible API; preserve exact original state and parameters. |
| `glTexParameterf` | `glTexParameterf` | Directly compatible API; preserve exact original state and parameters. |
| `glTranslatef` | `glTranslatef` | Directly compatible API; preserve exact original state and parameters. |
| `glVertexPointer` | `glVertexPointer` | Small adapter: direct native buffer pointers and lifetime; preserve GL_SHORT vertices, float color/UV, GL_UNSIGNED_BYTE indices. |
| `glViewport` | `glViewport` | Directly compatible API; preserve exact original state and parameters. |
| `glBindFramebufferOES` | `glBindFramebuffer` | Small adapter: drop OES suffix; translate GL constants; verify FBO orientation and completeness. |
| `glCheckFramebufferStatusOES` | `glCheckFramebufferStatus` | Small adapter: drop OES suffix; translate GL constants; verify FBO orientation and completeness. |
| `glDeleteFramebuffersOES` | `glDeleteFramebuffers` | Small adapter: drop OES suffix; translate GL constants; verify FBO orientation and completeness. |
| `glDeleteRenderbuffersOES` | `glDeleteRenderbuffers` | Small adapter: drop OES suffix; translate GL constants; verify FBO orientation and completeness. |
| `glFramebufferTexture2DOES` | `glFramebufferTexture2D` | Small adapter: drop OES suffix; translate GL constants; verify FBO orientation and completeness. |
| `glGenFramebuffersOES` | `glGenFramebuffers` | Small adapter: drop OES suffix; translate GL constants; verify FBO orientation and completeness. |
| `glGenRenderbuffersOES` | `glGenRenderbuffers` | Small adapter: drop OES suffix; translate GL constants; verify FBO orientation and completeness. |

## Behavior to preserve

- Original projection: scaled screen height 320; width truncates `width * 320 / height`.
  Centered orthographic projection, Z -100..100; content reference width 480,
  horizontal offset `(scaled_width - 480)/2`, max width field 568. On Vita
  960×544 this produces a scaled width of 564. Use the inverse for touch.
- `Graphics2D.flash` batches quads as triangles with GL_SHORT positions,
  GL_FLOAT UV/color arrays and GL_UNSIGNED_BYTE indices. vitaGL draw.c supports
  unsigned-byte indices; ffp.c supports short vertex attributes.
- Blend modes: SRC_ALPHA/ONE_MINUS_SRC_ALPHA; SRC_ALPHA/ONE;
  ONE/ONE_MINUS_SRC_ALPHA. Retain transitions and flush ordering.
- `AndroidGLTexture`: nearest when pixcel=true, linear otherwise; MODULATE;
  CLAMP_TO_EDGE in both axes. PNG decode replaces BitmapFactory/GLUtils.
  Native images retain straight/premultiplied state; confirmed Community14
  premultiplication uses GL_ONE to avoid multiplying twice. Exact translucent
  reference-image fidelity across all scenes remains pending.
- `offscreen` uses GL_OES_framebuffer_object; allocation defaults 1024×512,
  with a 512-wide alternative. Do not replace those calls with framebuffer 0.
- `StringTexture` uses Android Canvas/Paint/Typeface/Bitmap for generated text:
  is implemented with PVF-backed mutable native surfaces. The boot diagnostic
  font is separate. Visible glyph rectangles are restored in 00.19; exact
  Android metrics, complete script coverage and all dialogues remain pending.
- No glScissor or glAlphaFunc invocation was found in this DEX. Do not add
  alpha testing/scissor as an assumed renderer requirement.
- No custom gameplay shaders are required by the observed original calls.
  vitaGL still needs its runtime shader compiler (`libshacccg.suprx`).

## Implemented bridge ownership and performance

Java buffer position/limit and element types are honored at the native boundary.
Client arrays are copied into native-owned storage rather than retaining an
unsafe pointer across moving-GC activity. Keep original batching/flush order.
FBO calls target actual offscreen surfaces and maintain original texture state;
text upload, mutable surfaces and image decoding are distinct paths.

00.20's 4 MiB retained texture LRU uses content hash plus exact source comparison
and sampling mode. Live owners prevent idle eviction/deletion. It is not a total
GPU allocation cap. Mutable StringTexture/FBO storage is not content-cached.
PVF memory-font fallback, cached advances and dirty-row uploads remain, while
visible glyph coverage must still use scePvfGetCharImageRect.

00.18 reaches stable 60 FPS in the user's battle test but hides text; 00.19
restores visible text. Do not infer shader/pixel fidelity, zero selection pauses
or latest-build startup from those observations. vglInitExtended's false return
selects normal 960×544 operation here; treating it as initialization failure
caused the historical 00.03 rejection. See [FAILURES](FAILURES.md).

Implementation: tools/aot/engine/native/gles.cpp, native text service and
handwritten Android GL/Bitmap/Canvas adapters. [BUILD](BUILD.md) distinguishes
this full target from the earlier atlas renderer; [VALIDATION](VALIDATION.md)
labels mocked host checks and real-device results.

## Sources

- https://github.com/Rinnegatamante/vitaGL
- https://vitasdk.org/
- `evidence/apk_inventory.json`: actual DEX API call inventory.

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current hardware checkpoint — 00.24 (2026-10-05, America/Bogota):** the user
> confirms `DBTapBattle-Vita-00.24-Battle-Audio-Fix.vpk` works on the physical Vita
> after the Android14 battle-start crash. Runtime source `f5672d4d`, VPK SHA-256
> `0a156820a065a273a4ed24b064145fa1eed1dad72c44d8e03185f5e857dbf345`.
> The original PAC streaming repair remains; Ogg PCM now uses one exact allocation
> instead of transient vector doubling, with cache-only resource reclamation.
> The approved LiveArea is retained. This is a user-confirmed test checkpoint,
> not exhaustive character/profile/mode or long-session certification. Historical
> records keep their original artifact and evidence scope.
<!-- DBTB_CURRENT_CHECKPOINT:END -->
