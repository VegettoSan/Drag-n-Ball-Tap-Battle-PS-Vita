#include "dbtb_bridge.h"
#include "services.hpp"
#include "performance.hpp"
#include "log.hpp"

#include <psp2/audioout.h>
#include <psp2/kernel/threadmgr.h>
#include <vorbis/vorbisfile.h>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <list>
#include <array>
#include <string>
#include <vector>

namespace {
constexpr int kOutputRate = 48000;
constexpr int kFrames = 1024;
constexpr size_t kVoiceChannels = 3;
constexpr size_t kEffectChannels = 20;

struct Clip {
    std::vector<int16_t> pcm;
    int channels = 0;
    int rate = 0;
    size_t frame_count = 0;
    bool bandlimited = false;
    size_t frames() const { return frame_count; }
};
struct Voice {
    std::shared_ptr<Clip> clip;
    uint64_t phase = 0;       // 32.32 source-frame position
    uint64_t step = 0;        // 32.32 source frames per 48 kHz output frame
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
bool audio_init_failed = false;
std::atomic<uint32_t> clipped_samples{0}, overload_samples{0}, late_mix_blocks{0}, max_mix_us{0};
std::atomic<uint32_t> submission_gaps{0}, max_submission_gap_us{0};
// Audio-worker-only envelope; callers join the worker before disposing it.
float output_gain = 1.0f;

// Only character PCM needs this upsampler. BGM/effects keep their existing path.
// A 16-tap, 256-phase windowed sinc suppresses interpolation images above the
// source Nyquist frequency. Q14 keeps the real-time sum in bounded int32 math.
constexpr int kVoiceTaps = 16, kVoicePhases = 256, kVoiceScale = 16384;
using VoiceFilter = std::array<std::array<int16_t, kVoiceTaps>, kVoicePhases>;
const VoiceFilter& voiceFilter() {
    static const VoiceFilter filter = [] {
        VoiceFilter result{};
        constexpr double pi = 3.14159265358979323846;
        for (int phase = 0; phase < kVoicePhases; ++phase) {
            double coefficients[kVoiceTaps], total = 0;
            const double fraction = double(phase) / kVoicePhases;
            for (int tap = 0; tap < kVoiceTaps; ++tap) {
                const double x = tap - 7 - fraction;
                const double sinc = std::abs(x) < 1e-10 ? 1 : std::sin(pi * x) / (pi * x);
                coefficients[tap] = sinc * (0.5 + 0.5 * std::cos(pi * x / 8));
                total += coefficients[tap];
            }
            int sum = 0;
            for (int tap = 0; tap < kVoiceTaps; ++tap) {
                result[phase][tap] = static_cast<int16_t>(std::lround(coefficients[tap] * kVoiceScale / total));
                sum += result[phase][tap];
            }
            result[phase][7] += kVoiceScale - sum; // exact DC unity gain
        }
        return result;
    }();
    return filter;
}

uint32_t voiceHash(const uint8_t* data, size_t size) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < size; ++i) hash = (hash ^ data[i]) * 16777619u;
    return hash;
}
struct CachedVoice {
    uint32_t hash;
    std::vector<uint8_t> bytes;
    std::shared_ptr<Clip> clip;
    size_t cost;
};
std::list<CachedVoice> voice_cache;
size_t voice_cache_bytes = 0;
constexpr size_t kVoiceCacheBudget = 2u * 1024u * 1024u;

Voice makeVoice(std::shared_ptr<Clip> clip, float gain, bool loop) {
    Voice voice;
    voice.clip = std::move(clip);
    voice.gain = std::max(0.0f, std::min(2.0f, gain));
    voice.loop = loop;
    if (voice.clip && voice.clip->rate > 0)
        voice.step = (uint64_t(uint32_t(voice.clip->rate)) << 32) / uint64_t(kOutputRate);
    return voice;
}

std::string audioName(const char* raw) {
    if (!raw) return {};
    std::string name(raw);
    if (name.size() < 4 || name.substr(name.size() - 4) != ".ogg") name += ".ogg";
    return name;
}

std::shared_ptr<Clip> decodeOgg(const std::string& relative) {
    DbtbTimedScope timer(dbtb_performance().audio_decode_us);
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
    // Files are seekable: Vorbis knows the exact decoded frame count. Growing
    // a vector in 8 KiB steps temporarily keeps its old allocation alongside
    // a doubled replacement (the 00.23 Android14 battle-start bad_alloc).
    const ogg_int64_t frames = ov_pcm_total(&vf, -1);
    constexpr uint64_t kMaxOggBytes = 32u * 1024u * 1024u;
    if (frames <= 0 || uint64_t(frames) > kMaxOggBytes / (2u * clip->channels)) {
        ov_clear(&vf); return nullptr;
    }
    for (int link = 0; link < ov_streams(&vf); ++link) {
        const vorbis_info* part = ov_info(&vf, link);
        if (!part || part->channels != clip->channels || part->rate != clip->rate) {
            ov_clear(&vf); return nullptr;
        }
    }
    const size_t samples = size_t(frames) * size_t(clip->channels);
    dbtb_reclaimIdleResources();
    runtimeLog("Ogg decode: " + relative + " frames=" + std::to_string(frames) +
               " pcm_bytes=" + std::to_string(samples * sizeof(int16_t)));
    clip->pcm.resize(samples);
    size_t written = 0;
    int bitstream = 0;
    for (;;) {
        // One extra read verifies EOF without growing the allocated PCM.
        char end_probe[4];
        const size_t left = samples * sizeof(int16_t) - written;
        char* target = left ? reinterpret_cast<char*>(clip->pcm.data()) + written : end_probe;
        const int request = left ? int(std::min(size_t(8192), left)) : sizeof(end_probe);
        const long got = ov_read(&vf, target, request, 0, 2, 1, &bitstream);
        if (got == 0) break;
        if (got < 0 || size_t(got) > left || (got % (2 * clip->channels))) {
            ov_clear(&vf); return nullptr;
        }
        written += size_t(got);
    }
    ov_clear(&vf);
    if (written != samples * sizeof(int16_t)) return nullptr;
    clip->frame_count = size_t(frames);
    return clip;
}

std::shared_ptr<Clip> decodeVoiceBytes(const void* raw, int32_t size) {
    DbtbTimedScope timer(dbtb_performance().audio_decode_us);
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
    clip->frame_count = clip->pcm.size() / size_t(clip->channels);
    size_t source_rails = 0;
    int source_peak = 0;
    for (int16_t sample : clip->pcm) {
        source_peak = std::max(source_peak, std::abs(int(sample)));
        source_rails += sample == -32768 || sample == 32767;
    }
    // Do not open/write/close runtime.log for every voice on every character
    // switch (00.19 measured ~140-285 ms just in voice load/diagnostics).
    static unsigned diagnostics = 0;
    if (diagnostics++ < 3) runtimeLog("Voice PCM: rate=" + std::to_string(clip->rate) +
        " frames=" + std::to_string(clip->frames()) + " peak=" + std::to_string(source_peak) +
        " source_rail_samples=" + std::to_string(source_rails) + " resampler=sinc16");
    clip->bandlimited = clip->rate < kOutputRate;
    if (clip->bandlimited) (void)voiceFilter(); // initialize on the loading thread
    return clip->frames() ? clip : nullptr;
}

void mixVoice(Voice& v, int32_t& left, int32_t& right) {
    if (!v.clip || !v.clip->frames() || !v.step) return;
    const uint64_t total = uint64_t(v.clip->frames()) << 32;
    if (v.phase >= total) {
        if (!v.loop) { v.clip.reset(); return; }
        v.phase %= total;
    }

    const size_t frame0 = static_cast<size_t>(v.phase >> 32);
    size_t frame1 = frame0 + 1;
    if (frame1 >= v.clip->frames()) frame1 = v.loop ? 0 : frame0;
    // Q16 interpolation is already far above 16-bit PCM precision and avoids
    // a pair of 64-bit multiplies for every source mixed at 48 kHz. Keep the
    // 32.32 position so long BGM tracks still have ample integer range.
    const uint32_t frac = static_cast<uint32_t>((v.phase >> 16) & 0xffffu);
    const uint32_t inv = 65536u - frac;
    const size_t channels = size_t(v.clip->channels);
    const int16_t* a = &v.clip->pcm[frame0 * channels];
    const int16_t* b = &v.clip->pcm[frame1 * channels];

    const auto interpolate = [frac, inv](int32_t x, int32_t y) -> int32_t {
        return (x * static_cast<int32_t>(inv) + y * static_cast<int32_t>(frac)) >> 16;
    };
    int32_t sample_l, sample_r;
    if (v.clip->bandlimited) {
        const auto& coefficients = voiceFilter()[unsigned(v.phase >> 24) & 255u];
        int32_t sum_l = 0, sum_r = 0;
        const bool interior = frame0 >= 7 && frame0 + 8 < v.clip->frames();
        for (int tap = 0; tap < kVoiceTaps; ++tap) {
            int64_t position = int64_t(frame0) + tap - 7;
            if (!interior && v.loop) {
                position %= int64_t(v.clip->frames());
                if (position < 0) position += v.clip->frames();
            } else if (!interior) position = std::max<int64_t>(0, std::min<int64_t>(v.clip->frames() - 1, position));
            const int16_t* sample = &v.clip->pcm[size_t(position) * channels];
            sum_l += int32_t(sample[0]) * coefficients[tap];
            if (channels == 2) sum_r += int32_t(sample[1]) * coefficients[tap];
        }
        sample_l = sum_l / kVoiceScale;
        sample_r = channels == 2 ? sum_r / kVoiceScale : sample_l;
    } else {
        sample_l = interpolate(a[0], b[0]);
        sample_r = v.clip->channels == 2 ? interpolate(a[1], b[1]) : sample_l;
    }
    left += static_cast<int32_t>(sample_l * v.gain);
    right += static_cast<int32_t>(sample_r * v.gain);

    v.phase += v.step;
    if (v.phase >= total) {
        if (v.loop) v.phase %= total;
        else v.clip.reset();
    }
}

int audioThread(SceSize, void*) {
    alignas(64) int16_t buffer[kFrames * 2];
    uint64_t previous_submission = 0;
    while (audio_running.load()) {
        const uint64_t start = dbtb_timeUs();
        dbtb_mixAudio(buffer, kFrames);
        const uint32_t elapsed = static_cast<uint32_t>(dbtb_timeUs() - start);
        if (elapsed > uint32_t(kFrames * 1000000 / kOutputRate))
            late_mix_blocks.fetch_add(1, std::memory_order_relaxed);
        uint32_t previous = max_mix_us.load(std::memory_order_relaxed);
        while (previous < elapsed && !max_mix_us.compare_exchange_weak(
                   previous, elapsed, std::memory_order_relaxed)) {}
        const uint64_t submission = dbtb_timeUs();
        if (previous_submission) {
            const uint32_t gap = static_cast<uint32_t>(submission - previous_submission);
            if (gap > uint32_t(2 * kFrames * 1000000 / kOutputRate))
                submission_gaps.fetch_add(1, std::memory_order_relaxed);
            uint32_t old_gap = max_submission_gap_us.load(std::memory_order_relaxed);
            while (old_gap < gap && !max_submission_gap_us.compare_exchange_weak(
                       old_gap, gap, std::memory_order_relaxed)) {}
        }
        previous_submission = submission;
        if (sceAudioOutOutput(audio_port, buffer) < 0) break;
    }
    return 0;
}

bool ensureAudio() {
    if (audio_port >= 0) return true;
    // A persistent setup error must not reopen a port for every effect/voice.
    // Disposal resets this latch for a new engine session.
    if (audio_init_failed) return false;
    auto fail = [](const char* operation, int result) {
        char message[160];
        std::snprintf(message, sizeof(message), "Audio: %s failed: 0x%08x (%d)",
                      operation, static_cast<unsigned int>(result), result);
        runtimeLog(message);
        audio_init_failed = true;
        audio_running.store(false);
        if (audio_thread >= 0) sceKernelDeleteThread(audio_thread);
        audio_thread = -1;
        if (audio_port >= 0) sceAudioOutReleasePort(audio_port);
        audio_port = -1;
        return false;
    };
    audio_port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN, kFrames, kOutputRate, SCE_AUDIO_OUT_MODE_STEREO);
    if (audio_port < 0) return fail("sceAudioOutOpenPort", audio_port);
    runtimeLog("Audio: output port opened");
    // Restore the priority used by the working 00.19 build and the VitaSDK
    // creation example. The 00.20 change to 0x10000080 coincided with
    // worker-setup failures on hardware; do not infer a valid range from
    // the ordering of these encoded priority values.
    audio_thread = sceKernelCreateThread("DBTB audio", audioThread, 0x10000100, 0x10000, 0, 0, nullptr);
    if (audio_thread < 0) return fail("sceKernelCreateThread", audio_thread);
    audio_running.store(true);
    const int started = sceKernelStartThread(audio_thread, 0, nullptr);
    if (started < 0) return fail("sceKernelStartThread", started);
    runtimeLog("Audio: worker thread started");
    return true;
}
}

void dbtb_mixAudio(short* interleaved, int frames) {
    if (!interleaved || frames <= 0) return;
    // Release the state lock between small portions of the mixing workload so
    // game-thread commands do not wait for a whole output block's calculation.
    // These are source sample counts, not a claim about measured lock duration.
    constexpr int kMixChunk = 64;
    uint32_t clipped = 0, overloaded = 0;
    alignas(64) int32_t mixed[kFrames * 2];
    // Look at each already-generated output block before reducing it to PCM16.
    // A stereo-linked gain preserves the summed waveform instead of flattening
    // its peaks. Only overload reduces volume; release takes about 100 ms.
    constexpr float kRelease = 1.0f / (0.1f * kOutputRate);
    for (int block = 0; block < frames; block += kFrames) {
        const int count = std::min(kFrames, frames - block);
        int32_t peak = 0;
        for (int base = 0; base < count; base += kMixChunk) {
          const int end = std::min(count, base + kMixChunk);
          AudioLockGuard lock;
          for (int i = base; i < end; ++i) {
              int32_t left = 0, right = 0;
              mixVoice(bgm, left, right);
              for (auto& v : active_effects) mixVoice(v, left, right);
              for (auto& v : active_voices) mixVoice(v, left, right);
              overloaded += left < -32768 || left > 32767;
              overloaded += right < -32768 || right > 32767;
              mixed[i * 2] = left;
              mixed[i * 2 + 1] = right;
              peak = std::max(peak, std::max(std::abs(left), std::abs(right)));
          }
          active_effects.erase(std::remove_if(active_effects.begin(), active_effects.end(), [](const Voice& v){ return !v.clip; }), active_effects.end());
          active_voices.erase(std::remove_if(active_voices.begin(), active_voices.end(), [](const Voice& v){ return !v.clip; }), active_voices.end());
        }
        // Leave a few integer units for float rounding. No per-block allocation,
        // file access, extra hardware buffering or changes to source rate/pitch.
        const float target = peak > 32767 ? 32760.0f / peak : 1.0f;
        output_gain = std::min(output_gain, target);
        for (int i = 0; i < count; ++i) {
            output_gain += (target - output_gain) * kRelease;
            for (int channel = 0; channel < 2; ++channel) {
                const int32_t value = static_cast<int32_t>(mixed[i * 2 + channel] * output_gain);
                clipped += value < -32768 || value > 32767;
                interleaved[(block + i) * 2 + channel] = static_cast<int16_t>(
                    std::max<int32_t>(-32768, std::min<int32_t>(32767, value)));
            }
        }
    }
    clipped_samples.fetch_add(clipped, std::memory_order_relaxed);
    overload_samples.fetch_add(overloaded, std::memory_order_relaxed);
}

DbtbAudioStats dbtb_takeAudioStats() {
    DbtbAudioStats stats;
    stats.clipped_samples = clipped_samples.exchange(0, std::memory_order_relaxed);
    stats.overload_samples = overload_samples.exchange(0, std::memory_order_relaxed);
    stats.late_mix_blocks = late_mix_blocks.exchange(0, std::memory_order_relaxed);
    stats.max_mix_us = max_mix_us.exchange(0, std::memory_order_relaxed);
    stats.submission_gaps = submission_gaps.exchange(0, std::memory_order_relaxed);
    stats.max_submission_gap_us = max_submission_gap_us.exchange(0, std::memory_order_relaxed);
    return stats;
}

extern "C" {
int32_t dbtb_effectLoad(void* raw_name) {
    if (!raw_name || !ensureAudio()) return -1;
    auto clip = decodeOgg(audioName(static_cast<const char*>(raw_name)));
    if (!clip) return -1;
    AudioLockGuard lock;
    if (effects.size() >= kEffectChannels) return -1;
    const int32_t id = static_cast<int32_t>(effects.size());
    effects.push_back(std::move(clip));
    // Original SoundEffect.load() returns sound_count before incrementing it.
    return id;
}

void dbtb_effectPlay(int32_t id, float gain) {
    if (!ensureAudio()) return;
    AudioLockGuard lock;
    // SoundPool's platform sample handle is opaque, but TCBManajer addresses the
    // SoundEffect.soundPoolMap by zero-based logical IDs (se_00 -> 0, se_01 -> 1).
    if (id < 0 || size_t(id) >= effects.size() || !effects[size_t(id)]) return;
    if (active_effects.size() >= kEffectChannels) active_effects.erase(active_effects.begin());
    active_effects.push_back(makeVoice(effects[size_t(id)], gain, false));
}

void dbtb_effectStop(void) {
    AudioLockGuard lock;
    active_effects.clear();
}

int32_t dbtb_voiceLoad(void* data, int32_t size) {
    if (!ensureAudio()) return -1;
    if (!data || size <= 1) return -1;
    const auto* bytes = static_cast<const uint8_t*>(data);
    const uint32_t hash = voiceHash(bytes, size_t(size));
    std::shared_ptr<Clip> clip;
    for (auto it = voice_cache.begin(); it != voice_cache.end(); ++it) {
        if (it->hash != hash || it->bytes.size() != size_t(size) ||
            std::memcmp(it->bytes.data(), bytes, size_t(size)) != 0) continue;
        clip = it->clip;
        voice_cache.splice(voice_cache.begin(), voice_cache, it);
        ++dbtb_performance().voice_cache_hits;
        break;
    }
    if (!clip) {
        clip = decodeVoiceBytes(data, size);
        if (clip) {
            const size_t cost = size_t(size) + clip->pcm.capacity() * sizeof(int16_t);
            if (cost <= kVoiceCacheBudget) {
                while (voice_cache_bytes + cost > kVoiceCacheBudget) {
                    voice_cache_bytes -= voice_cache.back().cost; voice_cache.pop_back();
                }
                voice_cache.push_front({hash, std::vector<uint8_t>(bytes, bytes + size), clip, cost});
                voice_cache_bytes += cost;
            }
        }
    }
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
    // translate this like the platform SoundPool sample handle.
    if (id < 0 || size_t(id) >= streamed_voice_clips.size() || !streamed_voice_clips[size_t(id)]) return;
    // Android owns exactly three AudioTrack playback channels. If all three are
    // busy getAudioIndex() rejects the new request instead of creating overlap.
    if (active_voices.size() >= kVoiceChannels) return;
    active_voices.push_back(makeVoice(streamed_voice_clips[size_t(id)], gain, false));
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
    bgm = makeVoice(std::move(clip), gain, loop != 0);
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
    output_gain = 1.0f;
    voice_cache.clear();
    voice_cache_bytes = 0;
    audio_init_failed = false;
}
}
