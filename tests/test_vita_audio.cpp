// Host-only behavioral probe; Vita platform calls are mocked.
#include "../tools/aot/engine/native/vita_audio.cpp"
#include <cassert>
#include <cstdio>
void runtimeLog(const std::string&) {}
const GameVfs& dbtb_vfs(){static GameVfs vfs("/tmp/dbtb-probe");return vfs;}
std::shared_ptr<Clip> clip(std::initializer_list<int16_t> pcm,int rate=22050,int channels=1){
 auto c=std::make_shared<Clip>();c->pcm=pcm;c->rate=rate;c->channels=channels;c->frame_count=c->pcm.size()/channels;return c;
}
int main(){
 auto c=clip({-32768,32767});auto v=makeVoice(c,1,false);v.phase=uint64_t(1)<<31;
 int32_t l=0,r=0;mixVoice(v,l,r);assert(l==-1&&r==-1);
 auto stereo=clip({1000,-1000,3000,-3000},48000,2);v=makeVoice(stereo,1,false);
 l=r=0;mixVoice(v,l,r);assert(l==1000&&r==-1000);l=r=0;mixVoice(v,l,r);assert(l==3000&&r==-3000&&!v.clip);
 v=makeVoice(stereo,1,true);for(int i=0;i<10000;++i){l=r=0;mixVoice(v,l,r);assert(l==(i%2?3000:1000));assert(r==-l);}
 auto duration=std::make_shared<Clip>();duration->pcm.assign(2205,1234);duration->channels=1;duration->rate=22050;duration->frame_count=2205;
 v=makeVoice(duration,1,false);int frames=0;while(v.clip){l=r=0;mixVoice(v,l,r);assert(l==1234&&r==1234);++frames;}assert(frames==4800||frames==4801);
 v=makeVoice(clip({32767,-32768}),1,false);for(unsigned f: {0u,1u,16384u,32768u,65535u}){v.phase=uint64_t(f)<<16;l=r=0;mixVoice(v,l,r);assert(l>=-32768&&l<=32767);}
 std::vector<int16_t> data(4000,1000);assert(dbtb_voiceLoad(data.data(),data.size()*2)==1);
 for(int i=0;i<4;++i)dbtb_voicePlay(0,1);assert(active_voices.size()==3);
 short out[256];dbtb_mixAudio(out,128);for(auto value:out)assert(value==3000);
 dbtb_voiceRelease();assert(active_voices.empty()&&streamed_voice_clips.empty());
 bgm=makeVoice(clip({30000,30000},48000),1,true);active_effects.push_back(makeVoice(clip({30000,30000},48000),1,true));
 dbtb_mixAudio(out,128);for(auto value:out)assert(value==32767);auto stats=dbtb_takeAudioStats();assert(stats.clipped_samples==256);
 dbtb_audioDispose();puts("AUDIO PASS: fractional interpolation, extremes, 22050/48000 duration, stereo, loops, 3 voice channels, clipping metrics");
}
