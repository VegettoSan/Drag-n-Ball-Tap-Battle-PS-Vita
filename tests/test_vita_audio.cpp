// Host-only behavioral probe; Vita platform calls are mocked.
#include "../tools/aot/engine/native/vita_audio.cpp"
#include <cassert>
#include <cstdio>
std::vector<std::string> audio_test_logs;
void dbtb_reclaimIdleResources() {}
void runtimeLog(const std::string& message) {audio_test_logs.push_back(message);}
const GameVfs& dbtb_vfs(){static GameVfs vfs("/tmp/dbtb-probe");return vfs;}
std::shared_ptr<Clip> clip(std::initializer_list<int16_t> pcm,int rate=22050,int channels=1){
 auto c=std::make_shared<Clip>();c->pcm=pcm;c->rate=rate;c->channels=channels;c->frame_count=c->pcm.size()/channels;return c;
}
int main(){
 // Exercise worker setup and cleanup independently of DSP. These stubs inject
 // API errors, not Vita scheduling; the requested priority matches the working
 // hardware build and SDK example, without inventing a kernel priority range.
 auto reset_setup=[](){dbtb_audioDispose();audio_test_kernel::reset();audio_test_output::reset();audio_test_logs.clear();};
 reset_setup();assert(ensureAudio());assert(audio_test_kernel::priority==0x10000100);
 assert(audio_test_output::opens==1&&audio_test_kernel::creates==1&&audio_test_kernel::starts==1);
 assert(ensureAudio()&&audio_test_output::opens==1&&audio_test_kernel::starts==1);
 dbtb_audioDispose();assert(audio_test_output::releases==1&&audio_test_kernel::waits==1&&audio_test_kernel::deletes==1);
 reset_setup();audio_test_output::open_result=-17;assert(!ensureAudio());
 assert(audio_thread==-1&&audio_port==-1&&!audio_running.load());
 assert(audio_test_kernel::creates==0&&audio_test_output::releases==0);
 assert(audio_test_logs.back().find("sceAudioOutOpenPort failed: 0xffffffef (-17)")!=std::string::npos);
 assert(!ensureAudio()&&audio_test_output::opens==1);
 reset_setup();audio_test_kernel::create_result=-18;assert(!ensureAudio());
 assert(audio_thread==-1&&audio_port==-1&&!audio_running.load());
 assert(audio_test_kernel::starts==0&&audio_test_kernel::deletes==0&&audio_test_output::releases==1);
 assert(audio_test_logs.back().find("sceKernelCreateThread failed: 0xffffffee (-18)")!=std::string::npos);
 assert(!ensureAudio()&&audio_test_output::opens==1&&audio_test_kernel::creates==1);
 reset_setup();audio_test_kernel::start_result=-19;assert(!ensureAudio());
 assert(audio_thread==-1&&audio_port==-1&&!audio_running.load());
 assert(audio_test_kernel::starts==1&&audio_test_kernel::deletes==1&&audio_test_kernel::waits==0&&audio_test_output::releases==1);
 assert(audio_test_logs.back().find("sceKernelStartThread failed: 0xffffffed (-19)")!=std::string::npos);
 assert(!ensureAudio()&&audio_test_kernel::starts==1&&audio_test_output::opens==1);
 reset_setup();assert(ensureAudio());dbtb_audioDispose();
 puts("AUDIO SETUP PASS: ready reuse, port/create/start errors, exact diagnostics, cleanup, failure latch and disposal retry");
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
 dbtb_mixAudio(out,128);for(auto value:out)assert(value>=32759&&value<=32760);auto stats=dbtb_takeAudioStats();assert(stats.clipped_samples==0&&stats.overload_samples==256);
 dbtb_audioDispose();
 // Overlapping nonconstant signals retain their shape and stereo balance;
 // a hard clamp would turn the two positive peaks into identical samples.
 auto wave=clip({10000,-5000,20000,-10000,30000,-15000,-30000,15000},48000,2);
 bgm=makeVoice(wave,1,true);active_effects.push_back(makeVoice(wave,1,true));
 std::vector<short> large(2051*2);dbtb_mixAudio(large.data(),2051);
 assert(large[0]>10918&&large[0]<10922&&large[2]>21838&&large[2]<21842);
 assert(large[4]>32758&&large[4]<=32760&&large[6]>=-32760);
 for(size_t i=0;i<large.size();i+=2)assert(std::abs(large[i]+2*large[i+1])<=2);
 stats=dbtb_takeAudioStats();assert(stats.clipped_samples==0&&stats.overload_samples>0);
 // Release is bounded and gradual across calls, with no permanent attenuation.
 active_effects.clear();bgm=makeVoice(clip({1000},48000),1,true);
 dbtb_mixAudio(out,128);assert(out[0]>500&&out[0]<1000&&out[254]>out[0]);
 for(int i=0;i<500;++i)dbtb_mixAudio(out,128);assert(out[254]>=999&&out[254]<=1000);
 dbtb_audioDispose();
 bgm=makeVoice(clip({1234,-2345},48000),1,true);dbtb_mixAudio(out,128);assert(out[0]==1234&&out[2]==-2345);
 // Real RIFF chunk walking, including odd-sized metadata, must keep PCM16
 // samples byte-for-byte and use the declared source rate, not header bytes.
 std::vector<uint8_t> riff={'R','I','F','F',0,0,0,0,'W','A','V','E','J','U','N','K',1,0,0,0,42,0,'f','m','t',' ',16,0,0,0,1,0,1,0,0x22,0x56,0,0,0x44,0xac,0,0,2,0,16,0,'d','a','t','a',4,0,0,0,0,0x80,0xff,0x7f};
 auto decoded=decodeVoiceBytes(riff.data(),riff.size());assert(decoded&&decoded->rate==22050&&decoded->channels==1&&decoded->pcm==std::vector<int16_t>({-32768,32767}));
 riff.pop_back();assert(!decodeVoiceBytes(riff.data(),riff.size()));
 // Every polyphase kernel has exact DC gain and a proven int32 accumulation
 // bound even for worst-case signed PCM16 inputs.
 for(const auto& phase:voiceFilter()){int sum=0,absolute=0;for(int weight:phase){sum+=weight;absolute+=std::abs(weight);}assert(sum==kVoiceScale&&absolute<65536);}
 for(int channels:{1,2}){
  auto dc=clip({1234,1234},22050,channels);dc->bandlimited=true;v=makeVoice(dc,1,true);
  for(int i=0;i<10000;++i){l=r=0;mixVoice(v,l,r);assert(l==1234&&r==1234);}
 }
 auto edge=clip({-32768,32767,-32768,32767},22050);edge->bandlimited=true;
 for(int phase=0;phase<256;++phase){v=makeVoice(edge,1,false);v.phase=uint64_t(phase)<<24;l=r=0;mixVoice(v,l,r);assert(l==r&&std::abs(l)<65536);}
 // Measure the resampling image at 14050 Hz of an 8000 Hz / 22050 Hz source.
 // Ignore edge padding; both paths use the same sample clock and duration.
 auto sine=std::make_shared<Clip>();sine->rate=22050;sine->channels=1;sine->frame_count=22050;sine->pcm.resize(22050);
 const double pi=3.14159265358979323846;
 for(size_t i=0;i<sine->frames();++i)sine->pcm[i]=int16_t(10000*std::sin(2*pi*8000*i/22050));
 auto amplitude=[&](bool sinc,int frequency){sine->bandlimited=sinc;auto voice=makeVoice(sine,1,false);double real=0,imaginary=0;
  for(int i=0;i<48000;++i){int32_t left=0,right=0;mixVoice(voice,left,right);if(i<480||i>=47520)continue;double angle=2*pi*frequency*i/48000;real+=left*std::cos(angle);imaginary+=left*std::sin(angle);}
  return 2*std::sqrt(real*real+imaginary*imaginary)/47040;
 };
 double linear_image=amplitude(false,14050),sinc_image=amplitude(true,14050),sinc_tone=amplitude(true,8000);
 assert(sinc_image<linear_image/10&&sinc_tone>9500&&sinc_tone<10500);
 printf("RESAMPLER SPECTRUM: linear image %.2f, sinc image %.2f, sinc tone %.2f (image reduction %.1f dB)\n",linear_image,sinc_image,sinc_tone,20*std::log10(linear_image/sinc_image));
 // Reloads after releasing a bank reuse decoded clips, while hash collisions
 // still require byte equality and cannot substitute another voice.
 dbtb_audioDispose();std::vector<int16_t> voice_data(2205,1234);
 assert(dbtb_voiceLoad(voice_data.data(),voice_data.size()*2)==1);auto saved=streamed_voice_clips.front();dbtb_voiceRelease();
 assert(dbtb_voiceLoad(voice_data.data(),voice_data.size()*2)==1&&streamed_voice_clips.front()==saved);
 dbtb_voiceRelease();voice_data[0]=4321;voice_cache.front().hash=voiceHash(reinterpret_cast<uint8_t*>(voice_data.data()),voice_data.size()*2);
 assert(dbtb_voiceLoad(voice_data.data(),voice_data.size()*2)==1&&streamed_voice_clips.front()!=saved&&streamed_voice_clips.front()->pcm[0]==4321);
 assert(voice_cache_bytes<=kVoiceCacheBudget);
 dbtb_audioDispose();puts("AUDIO PASS: interpolation, duration, stereo, loops, 3 voices, peak limiter/no clipping, waveform shape, release, multi-block output, RIFF parsing");
}
