#pragma once
using SceSize=unsigned int;using SceUID=int;
inline int sceKernelDelayThread(unsigned int){return 0;}
namespace audio_test_kernel {
int create_result=1, start_result=0, creates=0, starts=0, deletes=0, waits=0, priority=0;
inline void reset(){create_result=1;start_result=0;creates=starts=deletes=waits=priority=0;}
}
inline int sceKernelCreateThread(const char*,int (*)(SceSize,void*),int priority,int,int,int,void*){
    ++audio_test_kernel::creates;audio_test_kernel::priority=priority;return audio_test_kernel::create_result;
}
inline int sceKernelStartThread(int,int,void*){++audio_test_kernel::starts;return audio_test_kernel::start_result;}
inline int sceKernelDeleteThread(int){++audio_test_kernel::deletes;return 0;}
inline int sceKernelWaitThreadEnd(int,void*,void*){++audio_test_kernel::waits;return 0;}
