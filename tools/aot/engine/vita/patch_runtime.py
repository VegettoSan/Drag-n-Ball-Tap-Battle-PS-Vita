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
    required = ['all.c', 'definitions.h', 'exceptions.h', 'memory.c', 'time.c', 'fiber.c', 'date.c', 'classes/org/teavm/runtime/ExceptionHandling.c', 'classes/org/teavm/runtime/GC.c']
    missing = [name for name in required if not (root / name).is_file()]
    if missing:
        parser.error('Not a TeaVM 0.12.3 C output directory; missing: ' + ', '.join(missing))

    # Vita/newlib is not a full POSIX target. Keep TeaVM out of its mmap/signal
    # Unix paths and provide the small services actually required by this game.
    replace_once(root / 'definitions.h', '#define TEAVM_UNIX 1',
                 '#if !defined(__vita__)\n    #define TEAVM_UNIX 1\n    #endif')
    replace_once(root / 'exceptions.h', '#define TEAVM_UNREACHABLE return;',
                 '#define TEAVM_UNREACHABLE __builtin_unreachable();')

    # The original engine catches Runtime exceptions inside TCBManajer.Run and
    # only logs Exception.toString(), which hides the TeaVM call site that caused
    # a device-only NPE. Print the TeaVM shadow-stack immediately before creating
    # the NullPointerException on Vita; normal exception propagation is unchanged.
    replace_once(root / 'classes/org/teavm/runtime/ExceptionHandling.c', '''void teavm_throwNullPointerException() {
    void* teavm_tmp_ptr_0;
    meth_otr_ExceptionHandling_throwException((teavm_tmp_ptr_0 = meth_otr_Allocator_allocate(&jl_NullPointerException_Cls), meth_jl_NullPointerException__init_(teavm_tmp_ptr_0), teavm_tmp_ptr_0));
}''', '''void teavm_throwNullPointerException() {
#if defined(__vita__)
    teavm_printString(u"[DBTB] NullPointerException TeaVM stack:\\n");
    meth_otr_ExceptionHandling_printStack();
#endif
    void* teavm_tmp_ptr_0;
    meth_otr_ExceptionHandling_throwException((teavm_tmp_ptr_0 = meth_otr_Allocator_allocate(&jl_NullPointerException_Cls), meth_jl_NullPointerException__init_(teavm_tmp_ptr_0), teavm_tmp_ptr_0));
}''')

    # Capture the failing allocation and managed GC budget without allocating
    # Java objects. This is diagnostic only; collection/fatal behavior stays intact.
    replace_once(root / 'classes/org/teavm/runtime/GC.c',
                 "            meth_otr_ExceptionHandling_printStack();\n            teavm_outOfMemory();",
                 """#if defined(__vita__)
            fprintf(stderr, "[Memory] managed OOM requested=%d gc_free=%d available=%llu max=%llu chunks=%d\\n",
                teavm_local_1, sfld_otr_GC_freeMemory,
                (unsigned long long) teavm_gc_availableBytes,
                (unsigned long long) teavm_gc_maxAvailableBytes, sfld_otr_GC_freeChunks);
#endif
            meth_otr_ExceptionHandling_printStack();
            teavm_outOfMemory();""")

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

    # NewsData keeps java.util.Date reachable in the real game. Vita newlib has
    # mktime/localtime_r but not GNU timegm, so provide the missing UTC calendar
    # conversion rather than dropping Date as the old input probe did.
    replace_once(root / 'date.c', '''#if TEAVM_WINDOWS
    #define timegm _mkgmtime
    #define localtime_r(a, b) localtime_s(b, a)
#endif
''', '''#if TEAVM_WINDOWS
    #define timegm _mkgmtime
    #define localtime_r(a, b) localtime_s(b, a)
#endif

#if defined(__vita__)
#define DBTB_VITA_DATE_BACKEND 1
static int64_t dbtb_floor_div(int64_t value, int64_t divisor) {
    int64_t q = value / divisor;
    int64_t r = value % divisor;
    return r < 0 ? q - 1 : q;
}
static int64_t dbtb_days_from_civil(int64_t year, unsigned month, unsigned day) {
    year -= month <= 2;
    const int64_t era = dbtb_floor_div(year, 400);
    const unsigned yoe = (unsigned) (year - era * 400);
    const unsigned doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + (int64_t) doe - 719468;
}
static time_t dbtb_vita_timegm(struct tm* value) {
    int64_t year = (int64_t) value->tm_year + 1900;
    int64_t month0 = value->tm_mon;
    const int64_t year_adjust = dbtb_floor_div(month0, 12);
    year += year_adjust;
    month0 -= year_adjust * 12;
    const unsigned month = (unsigned) month0 + 1;
    const int64_t days = dbtb_days_from_civil(year, month, 1) + (int64_t) value->tm_mday - 1;
    const int64_t seconds = days * 86400 + (int64_t) value->tm_hour * 3600
                          + (int64_t) value->tm_min * 60 + value->tm_sec;
    return (time_t) seconds;
}
#define timegm dbtb_vita_timegm
#endif
''')
    replace_once(root / 'date.c', '    #if TEAVM_UNIX\n        struct tm t;',
                 '    #if TEAVM_UNIX || defined(__vita__)\n        struct tm t;')

    print('TeaVM 0.12.3 runtime adapted for private PS Vita engine build (Date retained)')


if __name__ == '__main__':
    main()
