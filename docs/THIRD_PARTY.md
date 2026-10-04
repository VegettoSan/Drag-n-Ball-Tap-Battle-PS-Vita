# Third-party code in the test build

The port's original-source reconstruction is not being supplied as decompiled
commercial Java. Commercial game data is also excluded. Original assets are
provided separately by the user.

The native VPK statically links vitaGL and its graphics support libraries, plus
libpng/zlib and the VitaSDK C/C++ runtime. Renderer package version and other
installed packages are documented in BUILD.md. Public source:

- vitaGL: https://github.com/Rinnegatamante/vitaGL (LGPL-3.0).
  Compiled package revision: 2bdbe89. API reference also reviewed at cdbba423.
- vitaShaRK: https://github.com/Rinnegatamante/vitaShaRK
- SceShaccCgExt: https://github.com/GrapheneCt/SceShaccCgExt
- taiHEN: https://github.com/henkaku/taiHEN
- libmathneon: https://github.com/vitasdk/packages/tree/master/libmathneon
- libpng/zlib: https://github.com/vitasdk/packages/tree/master/libpng and
  https://github.com/vitasdk/packages/tree/master/zlib
- Toolchain/runtime: https://github.com/vitasdk

Full vitaGL GPL/LGPL license text is included under licenses/ and in the VPK.
The symbols/relink bundle includes all port object files and the link invocation;
users can build a modified compatible vitaGL/library package and relink the
bootstrap using the repository CMake project and a matching VitaSDK ABI. The
source repository is https://github.com/VegettoSan/Drag-n-Ball-Tap-Battle-PS-Vita.
Changes in this audit do not change upstream library licenses or grant rights
to original game content.
