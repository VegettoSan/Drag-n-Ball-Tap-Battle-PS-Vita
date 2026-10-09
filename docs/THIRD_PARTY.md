# Third-party tools, runtime and test-build notices

<!-- DBTB_DOC_STATUS:START -->
> **Repository status — 2026-10-09:** main contains prepared v1.2 (`01.02` / `DBTB01178`),
> retaining v1.1 and approved Vita controls; dialogues use touch. Exact v1.2 hardware
> retest is pending; published release is 1.1. [Current contract](CURRENT_RUNTIME_CONTRACT.md) · [Status](CURRENT_STATUS.md).
> Current guide; explicitly dated experiments and superseded decisions remain historical.
<!-- DBTB_DOC_STATUS:END -->

Checkpoint 00.21 / 2026-10-05. Upstream license texts are not changed by this
documentation update. The repository contains handwritten port source and
non-commercial evidence; generated APK-derived Java/JAR/C stays private. The
full-engine executable nonetheless contains original compiled game code.
Excluding asset data is not an assertion that this executable is wholly open source.

## Native dependencies

The full VPK links vitaGL/support libraries, libpng/zlib, libvorbisfile/libvorbis/
libogg, VitaSDK C/C++/pthread runtime and system stubs. Matching hard-float package
versions and build options are recorded in [BUILD](BUILD.md).

- [vitaGL](https://github.com/Rinnegatamante/vitaGL): package revision 2bdbe89;
  API mapping reviewed at cdbba423. GPL/LGPL texts supplied in `licenses/` and
  the current VPK's notice entries; consult upstream for component terms.
- [vitaShaRK](https://github.com/Rinnegatamante/vitaShaRK),
  [SceShaccCgExt](https://github.com/GrapheneCt/SceShaccCgExt),
  [taiHEN](https://github.com/henkaku/taiHEN).
- [VitaSDK packages](https://github.com/vitasdk/packages): libmathneon,
  libpng/zlib, Vorbis/ogg and their upstream licenses.
- [VitaSDK toolchain/runtime](https://github.com/vitasdk).

## Private AOT tools and generated runtime

- [dex2jar 2.4](https://github.com/pxb1988/dex2jar/releases/tag/v2.4): private
  APK bytecode conversion.
- ECJ 3.37.0 and JDK 17: private adapter/generator compilation.
- [TeaVM 0.12.3](https://github.com/konsoletyper/teavm): private AOT, generated
  runtime/classlib; Apache-2.0 upstream project. Tool/runtime licensing does
  not grant rights to original game bytecode.

The shared community libabc.so/SWB are format-comparison sources, not linked
Android dependencies. Their matching bytes establish lineage, not packager
identity or an assumed license for copying the archive's commercial content.
See [ANDROID14_APK](ANDROID14_APK.md).

## Actual delivered materials and distribution gap

The verified 00.21 VPK has eboot, param.sfo, THIRD_PARTY.md and vitaGL GPL/LGPL
text entries. Its packaged notice is the snapshot at build source `07222bb`;
editing this Markdown does not retroactively alter that delivered VPK.
The 00.21 symbols ZIP contains ELF, VELF, a README and artifact evidence.
**It does not contain all object files/link inputs and is not a complete relink
kit.** Earlier bootstrap relink claims apply only to those older bundles.

Before a public full-engine distribution, review all applicable generated-runtime/
classlib/native notices and whether corresponding sources/relink materials and
commercial-code distribution scope are appropriate. In particular, do not assert
TeaVM runtime notices are already separately bundled when the current archive
inventory shows only the entries above. This records unfinished packaging work;
no new public full-engine release or license-compliance certification is made.
The private build recipe is [BUILD](BUILD.md); source is this repository.


## 00.22 delivery checkpoint

00.22 retains the same five-entry VPK notice layout and native/generated-runtime
scope, built at c40ce0a. Its packaged THIRD_PARTY is the documentation snapshot
from that source, describing the earlier 00.21 inventory; this later note does
not alter the delivered bytes. The 00.22 symbols ZIP has ELF/VELF, README and
artifact evidence only and is not a complete relink kit. The notice/distribution
review above remains open; this is another private full-engine test delivery.
[Artifact evidence](evidence/vita_selection_filter_build_00.22.json).

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current checkpoint — v1.2:** see [current status](CURRENT_STATUS.md) and
> [runtime contract](CURRENT_RUNTIME_CONTRACT.md). Earlier build identities/results stay historical.
<!-- DBTB_CURRENT_CHECKPOINT:END -->
