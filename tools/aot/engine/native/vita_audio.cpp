#include "dbtb_bridge.h"
#include "services.hpp"
#include "log.hpp"

#include <psp2/audioout.h>
#include <psp2/kernel/threadmgr.h>
#include <vorbis/vorbisfile.h>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace {
constexpr int kOutputRate = 48000;
constexpr int kFrames = 1024;
constexpr size_t kVoiceChannels = 3;

struct Clip {
    std::vector<int16_t> pcm;
    int channels = 0;
    int rate = 0;
    size_t frames() const { return channels > 0 ? pcm.size() / size_t(channels) : 0; }
};
struct Voice {
    std::shared_ptr<Clip> clip;
    double position = 0.0;
    float gain = 1.0f;
    bool loop = false;
};

std::atomic_flag audio_lock = ATOMIC_FLAG_INIT;

struct AudioLockGuard {
    AudioLockGuard() {
        while (audio_lock.test_and_set(std::memory_order_acquire))
            sceKernelDelayThread(50);
    }
    ~AudioLockGuard() { audio_lock.clear(std::memory_order_release); }
    AudioLockGuard(const AudioLockGuard&) = delete;
    AudioLockGuard& operator=(const AudioLockGuard&) = delete;
};
std::vector<std::shared_ptr<Clip>> effects;
std::vector<std::shared_ptr<Clip>> streamed_voice_clips;
std::vector<Voice> active_effects;
std::vector<Voice> active_voices;
Voice bgm;
int audio_port = -1;
SceUID audio_thread = -1;
std::atomic<bool> audio_running{false};

std::string audioName(const char* raw) {
    if (!raw) return {};
    std::string name(raw);
    if (name.size() < 4 || name.substr(name.size() - 4) != ".ogg") name += ".ogg";
    return name;
}

std::shared_ptr<Clip> decodeOgg(const std::string& relative) {
    std::string path;
    if (!dbtb_vfs().resolve(relative, path)) return nullptr;
    OggVorbis_File vf{};
    if (ov_fopen(path.c_str(), &vf) != 0) return nullptr;
    vorbis_info* info = ov_info(&vf, -1);
    if (!info || (info->channels != 1 && info->channels != 2) || info->rate <= 0) {
        ov_clear(&vf);
        return nullptr;
    }
    auto clip = std::make_shared<Clip>();
    clip->channels = info->channels;
    clip->rate = static_cast<int>(info->rate);
    char buffer[8192];
    int bitstream = 0;
    for (;;) {
        const long got = ov_read(&vf, buffer, sizeof(buffer), 0, 2, 1, &bitstream);
        if (got == 0) break;
        if (got < 0) { ov_clear(&vf); return nullptr; }
        const size_t old = clip->pcm.size();
        clip->pcm.resize(old + size_t(got) / sizeof(int16_t));
        std::memcpy(clip->pcm.data() + old, buffer, size_t(got));
    }
    ov_clear(&vf);
    return clip->frames() ? clip : nullptr;
}

std::shared_ptr<Clip> decodeVoiceBytes(const void* raw, int32_t size) {
    if (!raw || size <= 1) return nullptr;
    const auto* data = static_cast<const uint8_t*>(raw);
    auto clip = std::make_shared<Clip>();

    // Some character packs contain RIFF PCM while the original AudioTrack path
    // can also hand us headerless 16-bit mono PCM. Support both contracts.
    if (size >= 44 && std::memcmp(data, "RIFF", 4) == 0 && std::memcmp(data + 8, "WAVE", 4) == 0) {
        int channels = 0, rate = 0, bits = 0;
        const uint8_t* pcm = nullptr;
        size_t pcm_size = 0;
        size_t p = 12;
        while (p + 8 <= size_t(size)) {
            const uint32_t n = uint32_t(data[p + 4]) | (uint32_t(data[p + 5]) << 8) |
                               (uint32_t(data[p + 6]) << 16) | (uint32_t(data[p + 7]) << 24);
            const size_t body = p + 8;
            if (body + n > size_t(size)) return nullptr;
            if (std::memcmp(data + p, "fmt ", 4) == 0 && n >= 16) {
                const int format = data[body] | (data[body + 1] << 8);
                channels = data[body + 2] | (data[body + 3] << 8);
                rate = int(uint32_t(data[body + 4]) | (uint32_t(data[body + 5]) << 8) |
                           (uint32_t(data[body + 6]) << 16) | (uint32_t(data[body + 7]) << 24));
                bits = data[body + 14] | (data[body + 15] << 8);
                if (format != 1) return nullptr;
            } else if (std::memcmp(data + p, "data", 4) == 0) {
                pcm = data + body;
                pcm_size = n;
            }
            p = body + n + (n & 1u);
        }
        if (!pcm || bits != 16 || (channels != 1 && channels != 2) || rate <= 0 || (pcm_size & 1)) return nullptr;
        clip->channels = channels;
        clip->rate = rate;
        clip->pcm.resize(pcm_size / 2);
        std::memcpy(clip->pcm.data(), pcm, pcm_size);
    } else {
        clip->channels = 1;
        clip->rate = 22050;
        clip->pcm.resize(size_t(size) / 2);
        std::memcpy(clip->pcm.data(), data, clip->pcm.size() * 2);
    }
    return clip->frames() ? clip : nullptr;
}

void mixVoice(Voice& v, int32_t& left, int32_t& right) {
    if (!v.clip || !v.clip->frames()) return;
    size_t frame = static_cast<size_t>(v.position);
    if (frame >= v.clip->frames()) {
        if (!v.loop) { v.clip.reset(); return; }
        v.position = std::fmod(v.position, double(v.clip->frames()));
        frame = static_cast<size_t>(v.position);
    }
    const int16_t* sample = &v.clip->pcm[frame * size_t(v.clip->channels)];
    const float gain = std::max(0.0f, std::min(2.0f, v.gain));
    const int32_t l = static_cast<int32_t>(sample[0] * gain);
    const int32_t r = static_cast<int32_t>((v.clip->channels == 2 ? sample[1] : sample[0]) * gain);
    left += l; right += r;
    v.position += double(v.clip->rate) / double(kOutputRate);
    if (v.position >= v.clip->frames() && v.loop)
        v.position = std::fmod(v.position, double(v.clip->frames()));
}

int audioThread(SceSize, void*) {
    alignas(64) int16_t buffer[kFrames * 2];
    while (audio_running.load()) {
        dbtb_mixAudio(buffer, kFrames);
        if (sceAudioOutOutput(audio_port, buffer) < 0) break;
    }
    return 0;
}

bool ensureAudio() {
    if (audio_port >= 0) return true;
    audio_port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN, kFrames, kOutputRate, SCE_AUDIO_OUT_MODE_STEREO);
    if (audio_port < 0) { runtimeLog("Audio: sceAudioOutOpenPort failed: " + std::to_string(audio_port)); return false; }
    runtimeLog("Audio: output port opened");
    audio_running.store(true);
    audio_thread = sceKernelCreateThread("DBTB audio", audioThread, 0x10000100, 0x10000, 0, 0, nullptr);
    if (audio_thread < 0 || sceKernelStartThread(audio_thread, 0, nullptr) < 0) {
        audio_running.store(false);
        if (audio_thread >= 0) sceKernelDeleteThread(audio_thread);
        audio_thread = -1;
        sceAudioOutReleasePort(audio_port);
        audio_port = -1;
        runtimeLog("Audio: worker thread start failed");
        return false;
    }
    runtimeLog("Audio: worker thread started");
    return true;
}
}

void dbtb_mixAudio(short* interleaved, int frames) {
    if (!interleaved || frames <= 0) return;
    AudioLockGuard lock;
    for (int i = 0; i < frames; ++i) {
        int32_t left = 0, right = 0;
        mixVoice(bgm, left, right);
        for (auto& v : active_effects) mixVoice(v, left, right);
        for (auto& v : active_voices) mixVoice(v, left, right);
        interleaved[i * 2] = static_cast<int16_t>(std::max<int32_t>(-32768, std::min<int32_t>(32767, left)));
        interleaved[i * 2 + 1] = static_cast<int16_t>(std::max<int32_t>(-32768, std::min<int32_t>(32767, right)));
    }
    active_effects.erase(std::remove_if(active_effects.begin(), active_effects.end(), [](const Voice& v){ return !v.clip; }), active_effects.end());
    active_voices.erase(std::remove_if(active_voices.begin(), active_voices.end(), [](const Voice& v){ return !v.clip; }), active_voices.end());
}

extern "C" {
int32_t dbtb_effectLoad(void* raw_name) {
    if (!raw_name || !ensureAudio()) return -1;
    auto clip = decodeOgg(audioName(static_cast<const char*>(raw_name)));
    if (!clip) return -1;
    AudioLockGuard lock;
    effects.push_back(std::move(clip));
    return static_cast<int32_t>(effects.size());
}

void dbtb_effectPlay(int32_t id, float gain) {
    if (!ensureAudio()) return;
    AudioLockGuard lock;
    if (id <= 0 || size_t(id) > effects.size() || !effects[size_t(id - 1)]) return;
    if (active_effects.size() >= 32) active_effects.erase(active_effects.begin());
    active_effects.push_back({effects[size_t(id - 1)], 0.0, gain, false});
}

void dbtb_effectStop(void) {
    AudioLockGuard lock;
    active_effects.clear();
}

int32_t dbtb_voiceLoad(void* data, int32_t size) {
    if (!ensureAudio()) return -1;
    auto clip = decodeVoiceBytes(data, size);
    if (!clip) return -1;
    AudioLockGuard lock;
    streamed_voice_clips.push_back(std::move(clip));
    // The original loadAudioTrack returns a positive count, but TCBManajer only
    // checks it for failure. Actual playback IDs come from iReqSENo-80 and are
    // zero-based indexes into SoundEffect.wave[].
    return static_cast<int32_t>(streamed_voice_clips.size());
}

void dbtb_voicePlay(int32_t id, float gain) {
    if (!ensureAudio()) return;
    AudioLockGuard lock;
    // Original SoundEffect.playAudio(int) indexes wave[id] directly. Do not
    // translate this like SoundPool IDs, which are one-based.
    if (id < 0 || size_t(id) >= streamed_voice_clips.size() || !streamed_voice_clips[size_t(id)]) return;
    // Android owns exactly three AudioTrack playback channels. If all three are
    // busy getAudioIndex() rejects the new request instead of creating overlap.
    if (active_voices.size() >= kVoiceChannels) return;
    active_voices.push_back({streamed_voice_clips[size_t(id)], 0.0, gain, false});
}

void dbtb_voiceRelease(void) {
    AudioLockGuard lock;
    active_voices.clear();
    streamed_voice_clips.clear();
}

void dbtb_voiceStop(void) {
    AudioLockGuard lock;
    active_voices.clear();
}

int32_t dbtb_bgmPlay(void* raw_name, float gain, int32_t loop) {
    if (!raw_name || !ensureAudio()) return -1;
    auto clip = decodeOgg(audioName(static_cast<const char*>(raw_name)));
    if (!clip) return -1;
    AudioLockGuard lock;
    bgm = {std::move(clip), 0.0, gain, loop != 0};
    return 0;
}

void dbtb_bgmStop(void) {
    AudioLockGuard lock;
    bgm = Voice{};
}

void dbtb_audioDispose(void) {
    audio_running.store(false);
    if (audio_thread >= 0) {
        sceKernelWaitThreadEnd(audio_thread, nullptr, nullptr);
        sceKernelDeleteThread(audio_thread);
        audio_thread = -1;
    }
    if (audio_port >= 0) {
        sceAudioOutReleasePort(audio_port);
        audio_port = -1;
    }
    AudioLockGuard lock;
    bgm = Voice{};
    active_effects.clear();
    active_voices.clear();
    effects.clear();
    streamed_voice_clips.clear();
}
}
