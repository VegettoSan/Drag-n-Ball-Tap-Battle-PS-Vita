#pragma once
#include <cstdint>
inline uint64_t sceKernelGetProcessTimeWide(){static uint64_t t=0;return ++t;}
