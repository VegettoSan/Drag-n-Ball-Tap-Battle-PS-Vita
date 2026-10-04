#!/usr/bin/env python3
"""Experimental TeaVM 0.12.3 input-probe runtime adaptation, not the game runtime.

Run only on a private copy of generated C. No generated game source belongs in Git.
"""
import argparse
from pathlib import Path


def replace_once(path, before, after):
    text = path.read_text()
    if text.count(before) != 1:
        raise ValueError(f'{path.name}: expected pinned runtime shape; regenerate from TeaVM 0.12.3')
    path.write_text(text.replace(before, after))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('generated_directory', type=Path)
    args = parser.parse_args()
    root = args.generated_directory
    replace_once(root / 'definitions.h', '#define TEAVM_UNIX 1',
                 '#if !defined(__vita__)\n    #define TEAVM_UNIX 1\n    #endif')
    replace_once(root / 'exceptions.h', '#define TEAVM_UNREACHABLE return;',
                 '#define TEAVM_UNREACHABLE __builtin_unreachable();')
    # The input probe uses System timers, not java.util.Date. Omit the unused
    # date backend needing GNU timegm. A future Date consumer must supply it;
    # unresolved symbols fail the link instead of receiving invented dates.
    replace_once(root / 'all.c', '#include "date.c"', '// unused Date backend omitted for input probe')
    replace_once(root / 'memory.c', 'static int64_t teavm_pageCount', '''
#if defined(__vita__)
// Reserve the configured bounded heap eagerly; Vita has no mmap/mprotect.
static void* teavm_virtualAlloc(int64_t size) {
    if (size < 0 || (uint64_t)size > SIZE_MAX) abort();
    void* data = calloc(1, (size_t)size);
    if (!data) abort();
    return data;
}
static void teavm_virtualCommit(void* address, int64_t size) { (void)address; (void)size; }
static void teavm_virtualUncommit(void* address, int64_t size) { (void)address; (void)size; }
static int64_t teavm_pageSize(void) { return 4096; }
#endif

static int64_t teavm_pageCount''')
    with (root / 'time.c').open('a') as f:
        f.write('''
#if defined(__vita__)
#include <sys/time.h>
#include <psp2/kernel/processmgr.h>
int64_t teavm_currentTimeMillis(void) {
    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0) abort();
    return (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
int64_t teavm_currentTimeNano(void) { return (int64_t)sceKernelGetSystemTimeWide() * 1000; }
#endif
''')
    with (root / 'fiber.c').open('a') as f:
        f.write('''
#if defined(__vita__)
#include <stdatomic.h>
#include <psp2/kernel/processmgr.h>
static atomic_int teavm_vita_interrupt;
void teavm_waitFor(int64_t timeout) {
    if (timeout < 0 || timeout > INT64_MAX / 1000) abort();
    const uint64_t start = sceKernelGetSystemTimeWide();
    const uint64_t delay = (uint64_t)timeout * 1000;
    while (!atomic_exchange(&teavm_vita_interrupt, 0)) {
        if (timeout && sceKernelGetSystemTimeWide() - start >= delay) return;
        sceKernelDelayThread(1000);
    }
}
void teavm_interrupt(void) { atomic_store(&teavm_vita_interrupt, 1); }
#endif
''')
    print('Input-probe runtime adapted; file I/O and full Android/game services are not provided')


if __name__ == '__main__':
    main()
