# Java core → native C: historical feasibility probes

InputProbe and the unadapted EngineProbe below are historical feasibility tests,
not release targets. The full original core is now the production port approach
in [engine/README.md](engine/README.md); use [BUILD](../../docs/BUILD.md) for it.
This document retains the early probe recipes/results and their limited scope.
No generated game JAR, class, C source, executable or commercial data is in Git.
The handwritten probes call classes supplied by the user's original APK.

## VirtualPadProbe — 2026-10-08 research

The additional `VirtualPadProbe.java` exercises actual original combat pad
types 1/4: eight directions, six buttons, press/hold/release, effective ranges,
five simultaneous IDs 0..4 and continuous direction changes. It passes on the
host JVM against the pinned original APK converted with dex2jar 2.4. Its new
TeaVM/native comparison and Vita integration remain pending; the historical
InputProbe AOT result below does not establish those new checks.
See [controls research](../../docs/VITA_CONTROLS_RESEARCH_2026-10-08.md) for
reproduction, evidence and the adapter-only implementation plan.

## Results

DEX→JAR succeeds with dex2jar 2.4 on original APK b84f98a3ed70957354f358b7930bd8fb651cc89b74e16f8774ebd989fbf0899b.
InputProbe tests original KeyData/Controller: begin/move/end, pointer identity,
ten-slot overflow/reuse and a type-5 button. JVM and TeaVM 0.12.3 native C produce
byte-for-byte equal output, including the local UTF-16 compatibility layer:

```text
press:16:0:16:1
move:0:0:16:2
release:0:16:0:0
UTF16: ñ 日本 🐉
ORIGINAL INPUT AOT PASS
```

The adapted probe compiles/links to ARM EABI5 hard-float and converts to VELF/SELF
with VitaSDK 2026.08.1-1/GCC 15.2. **Vita/Vita3K execution remains unverified.**
It is not connected to src/input.cpp or the VPK; it proves neither all gestures,
device GC/scheduler behavior nor the game loop/rendering/combat.

EngineProbe attempts original GlobalWork, TCBManajer.Init and Run. With the historical basic
Android 4.1.1.4 stub JAR and probe classpath, TeaVM emits 203 diagnostics (44
unique messages) and no full-engine C. Missing GL10/GL11ExtensionPack/JSON are
classpath gaps; network/security/reflection/platform boundaries need replacement.
On the JVM, Android's stub throws at BluetoothAdapter.getDefaultAdapter. These
results do not prove TeaVM cannot port the game; adding Android stubs does not
provide its services. Replace VFS/raw lookup, texture/render/text, lifecycle,
audio, save and obsolete online dependencies before retesting Init→Run→menu.
Original/community resource codecs exist in C++; code-modified APK behavior still
needs explicit recovery/adaptation. The later accepted production choice is original-core AOT with real platform
adapters (ADR-011 in [DECISIONS](../../docs/DECISIONS.md)); the failing unadapted
probe is preserved as a baseline, not the current engine status.

## Reproduction

Use a private directory outside tracked source. Versions used: Java 17, Maven
3.9.11, ECJ 3.37.0, dex2jar 2.4, TeaVM 0.12.3. Compile the probe against the
APK-derived JAR; it needs no Android JAR. A full JDK supplies jar; Python zipfile
can assemble the class files when only a JRE is available.

```sh
bash /path/to/d2j-dex2jar.sh --force --output /private/original.jar /path/to/original.apk
mvn -f tools/aot/pom.xml org.apache.maven.plugins:maven-dependency-plugin:3.8.1:copy-dependencies -DoutputDirectory=/private/lib
java -jar /path/to/ecj.jar -8 -d /private/classes -cp /private/original.jar tools/aot/InputProbe.java
jar cf /private/probe.jar -C /private/classes .
java -cp /private/probe.jar:/private/original.jar com.namcobandaigames.dragonballtap.apk.InputProbe > /private/jvm.txt
java -cp '/private/lib/*:/private/probe.jar:/private/original.jar' org.teavm.cli.TeaVMRunner -t c -d /private/c --min-heap 4 --max-heap 8 --strict -- com.namcobandaigames.dragonballtap.apk.InputProbe
cc -std=c11 -O2 /private/c/all.c -lm -lrt -o /private/input-native
/private/input-native > /private/native.txt
cmp /private/jvm.txt /private/native.txt
cp -r /private/c /private/c-vita
python tools/aot/vita/patch_runtime.py /private/c-vita
"$VITASDK/bin/arm-vita-eabi-gcc" -std=c11 -O2 -ffunction-sections -fdata-sections -Itools/aot/vita /private/c-vita/all.c tools/aot/vita/utf16.c tools/aot/vita/heap.c -Wl,-q,--gc-sections -lSceLibKernel_stub -lm -o /private/input-vita
"$VITASDK/bin/vita-elf-create" /private/input-vita /private/input.velf
"$VITASDK/bin/vita-make-fself" -s /private/input.velf /private/input.self
```

Configure Maven's proxy if required; it does not automatically use curl's proxy
variables. CLI uses its JVM classpath and `--` to avoid greedy -p/main parsing.
Compile all.c once, not main.c alone or all.c plus its constituent sources.
Do not add the generated root to -I: string.h/time.h shadow C library headers.

The patch requires a fresh private copy of the pinned runtime shape. Vita is
excluded from POSIX detection; bounded heap reservation uses calloc rather than
mmap/mprotect; wall-clock millis use gettimeofday, monotonic nanos the Vita
clock. Cooperative waits use short native delay polling and atomic interrupt.
GCC unreachable replaces invalid return fallback. Unused Date/GNU timegm is
omitted only for this probe; future Date consumers must implement it. File I/O
and Android services are absent; do not replace them with successful no-ops.
Keep the explicit 8-MiB heap cap; TeaVM's defaults are too large for this test.
heap.c reserves 16 MiB for the standalone process's Newlib heap; do not link it
alongside main.cpp, which already defines the production bootstrap heap.

Newlib lacks uchar.h. The local UTF-8↔UTF-16 implementation handles surrogate
pairs, partial input and invalid sequences. tests/test_aot_utf16.c passes host
ASan/UBSan with -Itools/aot/vita and tools/aot/vita/utf16.c. Those standalone tests do not establish device runtime/scheduling. The full
engine later implements lifecycle edges and cooperative EventQueue progress;
exhaustive device resume/restart coverage remains pending.

## References

- https://github.com/pxb1988/dex2jar/releases/tag/v2.4
- https://github.com/konsoletyper/teavm (Apache-2.0)
- https://teavm.org/docs/c-backend/getting-started.html
- https://github.com/soywiz-archive/jmedialayer-psvita-java at 10bf9ddbae5f0af34b189c426ba56d72c8506e55:
  historical Java/C++ Vita precedent, not a Tap Battle dependency or tested engine.

Future distribution containing TeaVM runtime must include its notices/license
and documented reproducible build. Only handwritten probes/adapters are committed.


## Follow-up: complete core generation

The unadapted EngineProbe result above remains a baseline. The handwritten
platform layer in [engine/README.md](engine/README.md) first generated the complete
reachable original Init/Run path with 456 classes/3989 methods. That historical count is superseded by later complete-core generation and full Vita link/package checks. The 00.34 hardware checkpoint confirms the modern platform path through Loading,
dynamic rosters, repeated Invasion fights, unified profile selection and the
final selector presentation. The exact tested 00.34 VPK is reported stable and
functional on physical Vita. The 00.21 startup/voice issues are historical, not current blockers. See [CURRENT_STATUS](../../docs/CURRENT_STATUS.md).
The original byte-array decoder and gameplay classes are preserved. Do not use
the input-only runtime patch unchanged for this full engine.

<!-- DBTB_00_23_DETAIL:START -->
## 00.23 AOT checkpoint

The full-engine AOT path now preserves the original streaming `GameData.Init`
resource parser and supplies Vita resources through `NativeResourceStream`. This
avoids a full PAC `byte[]` bridge allocation in the TeaVM heap. The resulting 00.23
runtime path passed the previously crashing battle-start transition on real hardware.
<!-- DBTB_00_23_DETAIL:END -->

<!-- DBTB_CURRENT_CHECKPOINT:START -->
> **Current hardware checkpoint — 00.34 (2026-10-07):** the exact
> `DBTapBattle-Vita-00.34-Button-Text-Center-Fix.vpk` is user-confirmed stable
> and functional on physical PS Vita for the exercised selector, profile-loading
> and gameplay paths, with no issue found so far. It retains the 00.33
> protected-PAC ownership fix and uses the unified `profiles-v1` data contract.
> See [CURRENT_STATUS](../../docs/CURRENT_STATUS.md).
<!-- DBTB_CURRENT_CHECKPOINT:END -->
