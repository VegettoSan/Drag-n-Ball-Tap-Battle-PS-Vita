#pragma once
#define SCE_AUDIO_OUT_PORT_TYPE_MAIN 0
#define SCE_AUDIO_OUT_MODE_STEREO 0
inline int sceAudioOutOpenPort(int,int,int,int){return 1;}
inline int sceAudioOutOutput(int,const void*){return 0;}
inline int sceAudioOutReleasePort(int){return 0;}
