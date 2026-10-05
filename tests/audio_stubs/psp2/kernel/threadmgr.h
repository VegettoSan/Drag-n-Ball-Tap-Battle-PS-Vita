#pragma once
using SceSize=unsigned int;using SceUID=int;
inline int sceKernelDelayThread(unsigned int){return 0;}
inline int sceKernelCreateThread(const char*,int (*)(SceSize,void*),int,int,int,int,void*){return 1;}
inline int sceKernelStartThread(int,int,void*){return 0;}
inline int sceKernelDeleteThread(int){return 0;}
inline int sceKernelWaitThreadEnd(int,void*,void*){return 0;}
