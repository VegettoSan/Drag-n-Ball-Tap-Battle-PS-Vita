#pragma once
#define SCE_AUDIO_OUT_PORT_TYPE_MAIN 0
#define SCE_AUDIO_OUT_MODE_STEREO 0
namespace audio_test_output {
int open_result=1, opens=0, releases=0;
inline void reset(){open_result=1;opens=releases=0;}
}
inline int sceAudioOutOpenPort(int,int,int,int){++audio_test_output::opens;return audio_test_output::open_result;}
inline int sceAudioOutOutput(int,const void*){return 0;}
inline int sceAudioOutReleasePort(int){++audio_test_output::releases;return 0;}
