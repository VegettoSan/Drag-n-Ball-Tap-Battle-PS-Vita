#!/usr/bin/env python3
"""Adapt a private TeaVM 0.12.3 C output directory for the PS Vita runtime.

This script mutates only the supplied generated directory. Run it on a private
copy: generated Dragon Ball Tap Battle C contains commercial game code and must
never be committed to this repository.
"""
import argparse
from pathlib import Path


def replace_once(path, before, after):
    text = path.read_text(encoding='utf-8')
    if text.count(before) != 1:
        raise ValueError(f'{path.name}: expected pinned TeaVM 0.12.3 runtime shape')
    path.write_text(text.replace(before, after), encoding='utf-8')


def append_once(path, marker, text):
    source = path.read_text(encoding='utf-8')
    if marker in source:
        raise ValueError(f'{path.name}: runtime already adapted')
    path.write_text(source + text, encoding='utf-8')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('generated_directory', type=Path)
    args = parser.parse_args()
    root = args.generated_directory.resolve()
    required = ['all.c', 'definitions.h', 'exceptions.h', 'memory.c', 'time.c', 'fiber.c', 'date.c']
    missing = [name for name in required if not (root / name).is_file()]
    if missing:
        parser.error('Not a TeaVM 0.12.3 C output directory; missing: ' + ', '.join(missing))

    # Vita/newlib is not a full POSIX target. Keep TeaVM out of its mmap/signal
    # Unix paths and provide the small services actually required by this game.
    replace_once(root / 'definitions.h', '#define TEAVM_UNIX 1',
                 '#if !defined(__vita__)\n    #define TEAVM_UNIX 1\n    #endif')
    replace_once(root / 'exceptions.h', '#define TEAVM_UNREACHABLE return;',
                 '#define TEAVM_UNREACHABLE __builtin_unreachable();')

    replace_once(root / 'memory.c', 'static int64_t teavm_pageCount', '''
#if defined(__vita__)
// TeaVM asks for a bounded max heap. Vita has no mmap/mprotect equivalent that
// matches the Unix backend, so reserve each region eagerly from Newlib.
static void* teavm_virtualAlloc(int64_t size) {
    if (size < 0 || (uint64_t) size > SIZE_MAX) abort();
    void* data = calloc(1, (size_t) size);
    if (!data) abort();
    return data;
}
static void teavm_virtualCommit(void* address, int64_t size) { (void) address; (void) size; }
static void teavm_virtualUncommit(void* address, int64_t size) { (void) address; (void) size; }
static int64_t teavm_pageSize(void) { return 4096; }
#endif

static int64_t teavm_pageCount''')

    append_once(root / 'time.c', 'DBTB_VITA_TIME_BACKEND', '''

#if defined(__vita__)
#define DBTB_VITA_TIME_BACKEND 1
#include <sys/time.h>
#include <psp2/kernel/processmgr.h>
int64_t teavm_currentTimeMillis(void) {
    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0) abort();
    return (int64_t) tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
int64_t teavm_currentTimeNano(void) {
    // Monotonic microseconds since boot; Java nanoTime requires duration use,
    // not wall-clock epoch semantics.
    return (int64_t) sceKernelGetSystemTimeWide() * 1000;
}
#endif
''')

    append_once(root / 'fiber.c', 'DBTB_VITA_FIBER_BACKEND', '''

#if defined(__vita__)
#define DBTB_VITA_FIBER_BACKEND 1
#include <stdatomic.h>
#include <psp2/kernel/processmgr.h>
static atomic_int teavm_vita_interrupt;
void teavm_waitFor(int64_t timeout) {
    if (timeout < 0 || timeout > INT64_MAX / 1000) abort();
    const uint64_t start = sceKernelGetSystemTimeWide();
    const uint64_t delay = (uint64_t) timeout * 1000;
    while (!atomic_exchange(&teavm_vita_interrupt, 0)) {
        if (timeout && sceKernelGetSystemTimeWide() - start >= delay) return;
        sceKernelDelayThread(1000);
    }
}
void teavm_interrupt(void) { atomic_store(&teavm_vita_interrupt, 1); }
#endif
''')

    # TeaVM always emits date.c into all.c even when java.util.Date was removed
    # by dependency analysis. Its Unix implementation needs GNU timegm, which
    # Vita newlib does not provide. Omit it only when no generated game/classlib
    # translation unit references the date runtime; otherwise fail loudly rather
    # than providing a fake calendar implementation.
    date_refs = []
    for path in root.rglob('*.c'):
        if path.name in {'all.c', 'date.c'}:
            continue
        if 'teavm_date_' in path.read_text(encoding='utf-8', errors='ignore'):
            date_refs.append(path.relative_to(root).as_posix())
    if date_refs:
        raise ValueError('Generated program uses java.util.Date; Vita date backend required: ' + ', '.join(date_refs[:8]))
    replace_once(root / 'all.c', '#include "date.c"',
                 '/* date.c omitted: generated program has no teavm_date_* references */')

    print('TeaVM 0.12.3 runtime adapted for private PS Vita engine build')


if __name__ == '__main__':
    main()
