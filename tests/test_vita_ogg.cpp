// Real Vorbis decoding and native PCM ownership; Vita output and cache calls
// are mocked. Private APK music is supplied as an external fixture directory.
#include <cstdlib>
#include <new>
#include <cstddef>
#include <cassert>
#include <cstdio>
namespace allocations {
bool recording = false;
unsigned epoch = 0;
size_t live = 0, peak = 0, largest = 0;
size_t request_limit = size_t(-1);
struct alignas(std::max_align_t) Header { size_t size; unsigned epoch; };
void start() { ++epoch; live = peak = largest = 0; recording = true; }
}
void* operator new(size_t size) {
    if (size > allocations::request_limit) throw std::bad_alloc();
    auto* h = static_cast<allocations::Header*>(std::malloc(sizeof(allocations::Header) + size));
    if (!h) throw std::bad_alloc();
    h->size = size; h->epoch = allocations::recording ? allocations::epoch : 0;
    if (h->epoch) {
        allocations::live += size;
        if (allocations::live > allocations::peak) allocations::peak = allocations::live;
        if (size > allocations::largest) allocations::largest = size;
    }
    return h + 1;
}
void operator delete(void* p) noexcept {
    if (!p) return;
    auto* h = static_cast<allocations::Header*>(p) - 1;
    if (h->epoch && h->epoch == allocations::epoch) allocations::live -= h->size;
    std::free(h);
}
void* operator new[](size_t size) { return ::operator new(size); }
void operator delete[](void* p) noexcept { ::operator delete(p); }
void operator delete(void* p, size_t) noexcept { ::operator delete(p); }
void operator delete[](void* p, size_t) noexcept { ::operator delete(p); }
#include "../tools/aot/engine/native/vita_audio.cpp"
std::string fixture_root;
unsigned reclaimed = 0;
const GameVfs& dbtb_vfs() { static GameVfs vfs(fixture_root); return vfs; }
void dbtb_reclaimIdleResources() { ++reclaimed; }
void runtimeLog(const std::string& message) { std::puts(message.c_str()); }

std::shared_ptr<Clip> legacy(const std::string& name) {
    std::string path; assert(dbtb_vfs().resolve(name, path));
    OggVorbis_File vf{}; assert(!ov_fopen(path.c_str(), &vf));
    const auto* info = ov_info(&vf, -1);
    auto clip = std::make_shared<Clip>(); clip->channels = info->channels; clip->rate = info->rate;
    char buffer[8192]; int bitstream = 0;
    try { for (;;) {
        const long got = ov_read(&vf, buffer, sizeof(buffer), 0, 2, 1, &bitstream);
        assert(got >= 0); if (!got) break;
        const size_t old = clip->pcm.size(); clip->pcm.resize(old + size_t(got) / 2);
        std::memcpy(clip->pcm.data() + old, buffer, got);
    } } catch (...) { ov_clear(&vf); throw; }
    ov_clear(&vf); clip->frame_count = clip->pcm.size() / clip->channels; return clip;
}
int main(int argc, char** argv) {
    assert(argc == 2); fixture_root = argv[1];
    allocations::request_limit = 6u * 1024u * 1024u;
    bool reproduced = false;
    try { legacy("bgm_03.ogg"); } catch (const std::bad_alloc&) { reproduced = true; }
    allocations::request_limit = size_t(-1);
    assert(reproduced);
    size_t avoided_peak = 0;
    for (int i = 0; i < 17; ++i) {
        char name[32]; std::snprintf(name, sizeof(name), "bgm_%02d.ogg", i);
        allocations::start(); auto reference = legacy(name);
        const auto old_peak = allocations::peak, old_largest = allocations::largest;
        allocations::recording = false;
        allocations::request_limit = 6u * 1024u * 1024u;
        allocations::start(); auto fixed = decodeOgg(name);
        allocations::recording = false;
        allocations::request_limit = size_t(-1);
        assert(fixed && fixed->pcm == reference->pcm && fixed->channels == reference->channels &&
               fixed->rate == reference->rate && fixed->frames() == reference->frames());
        const size_t bytes = fixed->pcm.size() * 2;
        assert(fixed->pcm.capacity() == fixed->pcm.size());
        assert(allocations::largest == bytes && allocations::peak < bytes + 4096);
        assert(old_peak > allocations::peak);
        avoided_peak = std::max(avoided_peak, old_peak - allocations::peak);
        printf("PCM PASS %s bytes=%zu legacy_peak=%zu exact_peak=%zu legacy_largest=%zu\n",
               name, bytes, old_peak, allocations::peak, old_largest);
        fixed.reset(); assert(allocations::live == 0);
    }
    assert(reclaimed == 17 && avoided_peak > 4u * 1024u * 1024u);
    assert(!decodeOgg("missing.ogg"));
    puts("OGG PASS: all 17 private BGM tracks preserve every PCM sample and frame; no vector growth, cache reclamation before allocation");
}
