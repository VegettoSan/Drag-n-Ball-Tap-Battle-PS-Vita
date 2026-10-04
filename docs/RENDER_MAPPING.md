# Original renderer → vitaGL mapping

Source: supplied DEX, disassembled with androguard and decompiled locally with
jadx 1.5.6. API availability checked against vitaGL commit
`cdbba4232cb93a741ba190be9a32143dfed12d8d`. Availability is source-level evidence,
**not a GPU fidelity or hardware test**.

| Original API | Vita equivalent | Classification / adaptation |
|---|---|---|
| `glBindTexture` | `glBindTexture` | Directly compatible API; preserve exact original state and parameters. |
| `glBlendFunc` | `glBlendFunc` | Directly compatible API; preserve exact original state and parameters. |
| `glClear` | `glClear` | Directly compatible API; preserve exact original state and parameters. |
| `glClearColor` | `glClearColor` | Directly compatible API; preserve exact original state and parameters. |
| `glColor4f` | `glColor4f` | Directly compatible API; preserve exact original state and parameters. |
| `glColorPointer` | `glColorPointer` | Small adapter: direct native buffer pointers and lifetime; preserve GL_SHORT vertices, float color/UV, GL_UNSIGNED_BYTE indices. |
| `glDeleteTextures` | `glDeleteTextures` | Small adapter: C pointer array (remove Java array offset). |
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
  Android premultiplied Bitmap behavior versus native straight RGBA needs
  reference-image validation for translucent assets (PENDING).
- `offscreen` uses GL_OES_framebuffer_object; allocation defaults 1024×512,
  with a 512-wide alternative. Do not replace those calls with framebuffer 0.
- `StringTexture` uses Android Canvas/Paint/Typeface/Bitmap for generated text:
  needs a font rasterizer service. The boot diagnostic font is not its substitute.
- No glScissor or glAlphaFunc invocation was found in this DEX. Do not add
  alpha testing/scissor as an assumed renderer requirement.
- No custom gameplay shaders are required by the observed original calls.
  vitaGL still needs its runtime shader compiler (`libshacccg.suprx`).

## Sources

- https://github.com/Rinnegatamante/vitaGL
- https://vitasdk.org/
- `evidence/apk_inventory.json`: actual DEX API call inventory.
