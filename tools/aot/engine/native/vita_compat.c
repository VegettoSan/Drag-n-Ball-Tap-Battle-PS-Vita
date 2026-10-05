#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// VitaSDK newlib does not export posix_memalign. The PVF allocator asks for
// 8-byte alignment; malloc already satisfies max_align_t on this ABI. Keep the
// compatibility symbol strict so a future caller asking for stronger alignment
// fails instead of receiving an incorrectly aligned pointer.
int posix_memalign(void** memptr, size_t alignment, size_t size) {
    if (!memptr || alignment < sizeof(void*) || (alignment & (alignment - 1)) != 0)
        return EINVAL;
    if (alignment > _Alignof(max_align_t))
        return EINVAL;
    void* ptr = malloc(size ? size : 1);
    if (!ptr)
        return ENOMEM;
    if (((uintptr_t) ptr & (alignment - 1)) != 0) {
        free(ptr);
        return EINVAL;
    }
    *memptr = ptr;
    return 0;
}
