#include "compressed_bgm.hpp"
#include "services.hpp"
#include "log.hpp"

#include <psp2/audiodec.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <climits>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <new>

namespace {
constexpr int kOutputRate = 48000;
constexpr size_t kMaxCompressedBytes = 16u * 1024u * 1024u;
constexpr size_t kMaxDecodedSamples = SCE_AUDIODEC_AAC_MAX_SAMPLES * 2u;

enum class Codec { Mp3, AacM4a };

struct Mp3Header {
    unsigned version = 0;
    unsigned rate = 0;
    unsigned channels = 0;
    unsigned frame_bytes = 0;
};

struct AacSample {
    uint32_t offset = 0;
    uint32_t size = 0;
};

uint32_t be32(const uint8_t* p) {
    return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) |
           (uint32_t(p[2]) << 8) | uint32_t(p[3]);
}
uint64_t be64(const uint8_t* p) {
    return (uint64_t(be32(p)) << 32) | be32(p + 4);
}

bool loadFile(const std::string& relative, std::vector<uint8_t>& bytes, std::string& error) {
    std::string path;
    if (!dbtb_vfs().resolve(relative, path)) {
        error = dbtb_vfs().error();
        return false;
    }
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) { error = "compressed BGM open failed"; return false; }
    if (std::fseek(f, 0, SEEK_END) != 0) { std::fclose(f); error = "compressed BGM seek failed"; return false; }
    const long size = std::ftell(f);
    if (size <= 0 || size_t(size) > kMaxCompressedBytes) {
        std::fclose(f); error = "compressed BGM size out of bounds"; return false;
    }
    if (std::fseek(f, 0, SEEK_SET) != 0) { std::fclose(f); error = "compressed BGM rewind failed"; return false; }
    dbtb_reclaimIdleResources();
    bytes.resize(size_t(size));
    const size_t got = std::fread(bytes.data(), 1, bytes.size(), f);
    std::fclose(f);
    if (got != bytes.size()) { bytes.clear(); error = "compressed BGM read failed"; return false; }
    return true;
}

bool mp3Header(const uint8_t* d, size_t len, Mp3Header& out) {
    if (len < 4 || d[0] != 0xff || (d[1] & 0xe0) != 0xe0) return false;
    const unsigned ver = (d[1] >> 3) & 3u;
    const unsigned layer = (d[1] >> 1) & 3u;
    const unsigned bri = (d[2] >> 4) & 15u;
    const unsigned sri = (d[2] >> 2) & 3u;
    if (ver == 1 || layer != 1 || bri == 0 || bri == 15 || sri == 3) return false;
    static const unsigned rates[4][3] = {
        {11025,12000,8000}, {0,0,0}, {22050,24000,16000}, {44100,48000,32000}
    };
    static const unsigned br1[16] = {0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,0};
    static const unsigned br2[16] = {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0};
    const unsigned rate = rates[ver][sri];
    const unsigned kbps = ver == 3 ? br1[bri] : br2[bri];
    if (!rate || !kbps) return false;
    const unsigned frame = ((ver == 3 ? 144000u : 72000u) * kbps) / rate + ((d[2] >> 1) & 1u);
    if (frame < 4 || frame > len) return false;
    out.version = ver;
    out.rate = rate;
    out.channels = ((d[3] >> 6) & 3u) == 3 ? 1u : 2u;
    out.frame_bytes = frame;
    return true;
}

bool findMp3(const std::vector<uint8_t>& bytes, size_t& start, Mp3Header& header) {
    size_t begin = 0;
    if (bytes.size() >= 10 && std::memcmp(bytes.data(), "ID3", 3) == 0) {
        const size_t tag = 10u + ((size_t(bytes[6] & 0x7f) << 21) |
                                  (size_t(bytes[7] & 0x7f) << 14) |
                                  (size_t(bytes[8] & 0x7f) << 7) |
                                  size_t(bytes[9] & 0x7f));
        if (tag < bytes.size()) begin = tag;
    }
    const size_t end = std::min(bytes.size(), begin + 64u * 1024u);
    for (size_t p = begin; p + 4 <= end; ++p) {
        Mp3Header first;
        if (!mp3Header(bytes.data() + p, bytes.size() - p, first)) continue;
        if (p + first.frame_bytes + 4 <= bytes.size()) {
            Mp3Header second;
            if (!mp3Header(bytes.data() + p + first.frame_bytes,
                           bytes.size() - p - first.frame_bytes, second)) continue;
            if (second.version != first.version || second.rate != first.rate || second.channels != first.channels)
                continue;
        }
        start = p; header = first; return true;
    }
    return false;
}

struct Box { size_t body = 0, end = 0; };

bool nextBox(const std::vector<uint8_t>& d, size_t& pos, size_t end, uint32_t& type, Box& box) {
    if (pos + 8 > end || end > d.size()) return false;
    uint64_t size = be32(d.data() + pos);
    type = be32(d.data() + pos + 4);
    size_t header = 8;
    if (size == 1) {
        if (pos + 16 > end) return false;
        size = be64(d.data() + pos + 8); header = 16;
    } else if (size == 0) {
        size = end - pos;
    }
    if (size < header || size > uint64_t(end - pos)) return false;
    box.body = pos + header;
    box.end = pos + size_t(size);
    pos = box.end;
    return true;
}

constexpr uint32_t fourcc(char a,char b,char c,char d) {
    return (uint32_t(uint8_t(a))<<24)|(uint32_t(uint8_t(b))<<16)|
           (uint32_t(uint8_t(c))<<8)|uint32_t(uint8_t(d));
}

bool childBox(const std::vector<uint8_t>& d, const Box& parent, uint32_t wanted, Box& found) {
    size_t pos = parent.body;
    while (pos < parent.end) {
        uint32_t type = 0; Box b;
        if (!nextBox(d, pos, parent.end, type, b)) return false;
        if (type == wanted) { found = b; return true; }
    }
    return false;
}

bool parseM4a(const std::vector<uint8_t>& d, size_t physical_bytes, int& channels, int& rate,
              std::vector<AacSample>& samples, std::string& error) {
    if (d.size() < 16 || std::memcmp(d.data() + 4, "ftyp", 4) != 0) return false;
    Box root{0,d.size()}, moov, trak, mdia, minf, stbl, stsd, stsz, stsc, stco, co64, mdhd;
    if (!childBox(d, root, fourcc('m','o','o','v'), moov) ||
        !childBox(d, moov, fourcc('t','r','a','k'), trak) ||
        !childBox(d, trak, fourcc('m','d','i','a'), mdia) ||
        !childBox(d, mdia, fourcc('m','i','n','f'), minf) ||
        !childBox(d, minf, fourcc('s','t','b','l'), stbl) ||
        !childBox(d, stbl, fourcc('s','t','s','d'), stsd) ||
        !childBox(d, stbl, fourcc('s','t','s','z'), stsz) ||
        !childBox(d, stbl, fourcc('s','t','s','c'), stsc)) {
        error = "AAC M4A missing required sample tables"; return false;
    }
    const bool have_stco = childBox(d, stbl, fourcc('s','t','c','o'), stco);
    const bool have_co64 = !have_stco && childBox(d, stbl, fourcc('c','o','6','4'), co64);
    if (!have_stco && !have_co64) { error = "AAC M4A missing chunk offsets"; return false; }

    if (stsd.body + 16 > stsd.end || be32(d.data()+stsd.body+4) < 1) {
        error = "AAC M4A invalid stsd"; return false;
    }
    const size_t entry = stsd.body + 8;
    if (entry + 36 > stsd.end || be32(d.data()+entry+4) != fourcc('m','p','4','a')) {
        error = "AAC M4A first sample entry is not mp4a"; return false;
    }
    channels = int((uint16_t(d[entry+24]) << 8) | d[entry+25]);
    rate = int(be32(d.data()+entry+32) >> 16);
    if ((channels != 1 && channels != 2) || rate <= 0) {
        error = "AAC M4A unsupported channel/rate"; return false;
    }
    if (childBox(d, mdia, fourcc('m','d','h','d'), mdhd) && mdhd.body + 16 <= mdhd.end) {
        const uint8_t version = d[mdhd.body];
        const size_t tpos = mdhd.body + (version == 1 ? 20 : 12);
        if (tpos + 4 <= mdhd.end) {
            const uint32_t timescale = be32(d.data()+tpos);
            if (timescale && timescale != uint32_t(rate)) {
                error = "AAC M4A rate/timescale mismatch"; return false;
            }
        }
    }

    if (stsz.body + 12 > stsz.end) { error = "AAC M4A truncated stsz"; return false; }
    const uint32_t fixed_size = be32(d.data()+stsz.body+4);
    const uint32_t sample_count = be32(d.data()+stsz.body+8);
    if (!sample_count || sample_count > 200000) { error = "AAC M4A sample count out of bounds"; return false; }
    std::vector<uint32_t> sizes;
    sizes.reserve(sample_count);
    if (fixed_size) {
        if (fixed_size > SCE_AUDIODEC_AAC_MAX_ES_SIZE) { error = "AAC sample exceeds Vita decoder ES limit"; return false; }
        sizes.assign(sample_count, fixed_size);
    } else {
        if (stsz.body + 12u + uint64_t(sample_count)*4u > stsz.end) { error = "AAC M4A truncated sample sizes"; return false; }
        for (uint32_t i=0;i<sample_count;++i) {
            const uint32_t size = be32(d.data()+stsz.body+12u+i*4u);
            if (!size || size > SCE_AUDIODEC_AAC_MAX_ES_SIZE) { error = "AAC sample size invalid for Vita decoder"; return false; }
            sizes.push_back(size);
        }
    }

    if (stsc.body + 8 > stsc.end) { error = "AAC M4A truncated stsc"; return false; }
    const uint32_t stsc_count = be32(d.data()+stsc.body+4);
    if (!stsc_count || stsc_count > 4096 || stsc.body + 8u + uint64_t(stsc_count)*12u > stsc.end) {
        error = "AAC M4A invalid stsc"; return false;
    }
    struct Stsc { uint32_t first, per; };
    std::vector<Stsc> map;
    map.reserve(stsc_count);
    for (uint32_t i=0;i<stsc_count;++i) {
        const size_t p = stsc.body + 8u + i*12u;
        const uint32_t first = be32(d.data()+p);
        const uint32_t per = be32(d.data()+p+4);
        const uint32_t desc = be32(d.data()+p+8);
        if (!first || !per || desc != 1 || (!map.empty() && first <= map.back().first)) {
            error = "AAC M4A unsupported stsc mapping"; return false;
        }
        map.push_back({first,per});
    }

    std::vector<uint64_t> chunks;
    const Box& cb = have_stco ? stco : co64;
    if (cb.body + 8 > cb.end) { error = "AAC M4A truncated chunk table"; return false; }
    const uint32_t chunk_count = be32(d.data()+cb.body+4);
    const size_t width = have_stco ? 4 : 8;
    if (!chunk_count || chunk_count > 100000 || cb.body + 8u + uint64_t(chunk_count)*width > cb.end) {
        error = "AAC M4A invalid chunk count"; return false;
    }
    chunks.reserve(chunk_count);
    for (uint32_t i=0;i<chunk_count;++i)
        chunks.push_back(have_stco ? be32(d.data()+cb.body+8u+i*4u) : be64(d.data()+cb.body+8u+i*8u));

    samples.clear(); samples.reserve(sample_count);
    size_t sample = 0, map_index = 0;
    for (uint32_t chunk=1; chunk<=chunk_count && sample<sample_count; ++chunk) {
        while (map_index + 1 < map.size() && chunk >= map[map_index+1].first) ++map_index;
        uint64_t off = chunks[chunk-1];
        for (uint32_t j=0;j<map[map_index].per && sample<sample_count;++j,++sample) {
            const uint32_t size = sizes[sample];
            if (off > physical_bytes || size > physical_bytes - size_t(off)) { error = "AAC M4A sample outside file"; return false; }
            samples.push_back({uint32_t(off),size});
            off += size;
        }
    }
    if (samples.size() != sample_count) { error = "AAC M4A sample mapping incomplete"; samples.clear(); return false; }
    return true;
}

// Only index metadata (ftyp/moov) is retained; mdat becomes an eight-byte
// placeholder. Original stco/co64 offsets still refer to the on-disk file.
bool readM4aIndex(FILE* f, size_t length, std::vector<uint8_t>& index,
                  std::string& error) {
    constexpr size_t kIndexBudget = 1024u * 1024u;
    index.clear();
    if (length < 16 || length > kMaxCompressedBytes) {
        error = "M4A source length outside supported range"; return false;
    }
    size_t pos = 0; bool have_ftyp = false, have_moov = false;
    for (unsigned i=0; pos + 8 <= length && i < 64; ++i) {
        if (pos > LONG_MAX || std::fseek(f, long(pos), SEEK_SET) != 0) {
            error = "M4A index seek failed"; return false;
        }
        uint8_t header[16]{};
        if (std::fread(header,1,8,f) != 8) { error = "M4A index read failed";return false; }
        uint64_t n = be32(header), width = 8;
        if (n == 1) {
            if (pos+16>length || std::fread(header+8,1,8,f)!=8) {
                error = "M4A extended box header truncated";return false;
            }
            n = be64(header+8);width = 16;
        } else if (n == 0) n=length-pos;
        if (n<width || n>length-pos || n>LONG_MAX) {
            error = "M4A top-level box outside source"; return false;
        }
        const uint32_t kind=be32(header+4);
        const bool copy=kind==fourcc('f','t','y','p') || kind==fourcc('m','o','o','v');
        if (copy) {
            if (kind==fourcc('f','t','y','p')) {
                if (pos!=0 || have_ftyp) { error="M4A has misplaced ftyp"; return false; }
                have_ftyp=true;
            }
            if (kind==fourcc('m','o','o','v')) {
                if (have_moov) { error="M4A has duplicate moov"; return false; }
                have_moov=true;
            }
            if (n>kIndexBudget-index.size()) { error="M4A index budget exceeded"; return false; }
            const size_t at=index.size();index.resize(at+size_t(n));
            if (std::fseek(f,long(pos),SEEK_SET)!=0 ||
                std::fread(index.data()+at,1,size_t(n),f)!=n) {
                error="M4A index payload truncated";return false;
            }
        } else {
            if (8>kIndexBudget-index.size()) { error="M4A index budget exceeded"; return false; }
            index.insert(index.end(),{0,0,0,8,header[4],header[5],header[6],header[7]});
        }
        pos+=size_t(n);
    }
    if (pos!=length || !have_ftyp || !have_moov) {
        error="M4A incomplete top-level index"; return false;
    }
    return true;
}

bool initLibrary(Codec codec, std::string& error) {
    static bool mp3_ready = false, aac_ready = false;
    bool& ready = codec == Codec::Mp3 ? mp3_ready : aac_ready;
    if (ready) return true;
    SceAudiodecInitParam p{};
    if (codec == Codec::Mp3) {
        p.mp3.size = sizeof(SceAudiodecInitStreamParam); p.mp3.totalStreams = 1;
    } else {
        p.aac.size = sizeof(SceAudiodecInitStreamParam); p.aac.totalStreams = 1;
    }
    const uint32_t type = codec == Codec::Mp3 ? SCE_AUDIODEC_TYPE_MP3 : SCE_AUDIODEC_TYPE_AAC;
    const int r = sceAudiodecInitLibrary(type, &p);
    if (r < 0 && uint32_t(r) != uint32_t(SCE_AUDIODEC_ERROR_ALREADY_INITIALIZED)) {
        char text[96]; std::snprintf(text,sizeof(text),"sceAudiodecInitLibrary failed 0x%08x",unsigned(r)); error=text; return false;
    }
    ready = true; return true;
}
} // namespace

struct DbtbCompressedBgm {
    Codec codec = Codec::Mp3;
    std::vector<uint8_t> bytes; // Original MP3 path; empty for indexed AAC.
    FILE* indexed_file = nullptr;
    size_t physical_bytes = 0;
    std::array<uint8_t,64u*1024u> read_ahead{};
    size_t read_base = 0, read_count = 0;
    std::array<uint8_t,SCE_AUDIODEC_AAC_MAX_ES_SIZE> aac_es{};
    std::vector<AacSample> aac_samples;
    ~DbtbCompressedBgm() { if (indexed_file) std::fclose(indexed_file); }
    size_t aac_index = 0;
    size_t mp3_start = 0, mp3_pos = 0;
    unsigned mp3_version = 0;
    SceAudiodecCtrl ctrl{};
    SceAudiodecInfo info{};
    bool decoder_created = false;
    bool loop = false;
    bool primed = false;
    bool eof_after_next = false;
    int channels = 0, rate = 0;
    float gain = 1.0f;
    uint64_t phase = 0, step = 0;
    std::array<int16_t,kMaxDecodedSamples> decoded{};
    size_t decoded_frame = 0, decoded_frames = 0;
    int16_t current_l = 0, current_r = 0, next_l = 0, next_r = 0;
    unsigned consecutive_errors = 0;
};

namespace {
bool createDecoder(DbtbCompressedBgm& s, std::string& error) {
    if (!initLibrary(s.codec,error)) return false;
    std::memset(&s.ctrl,0,sizeof(s.ctrl));
    std::memset(&s.info,0,sizeof(s.info));
    s.ctrl.size = sizeof(s.ctrl);
    s.ctrl.pInfo = &s.info;
    s.ctrl.wordLength = SCE_AUDIODEC_WORD_LENGTH_16BITS;
    s.ctrl.pPcm = s.decoded.data();
    s.ctrl.maxPcmSize = uint32_t(s.decoded.size()*sizeof(int16_t));
    uint32_t type = 0;
    if (s.codec == Codec::Mp3) {
        s.info.mp3.size = sizeof(SceAudiodecInfoMp3);
        s.info.mp3.ch = uint32_t(s.channels);
        s.info.mp3.version = s.mp3_version;
        s.ctrl.maxEsSize = SCE_AUDIODEC_MP3_MAX_ES_SIZE;
        type = SCE_AUDIODEC_TYPE_MP3;
    } else {
        s.info.aac.size = sizeof(SceAudiodecInfoAac);
        s.info.aac.isAdts = 0;
        s.info.aac.ch = uint32_t(s.channels);
        s.info.aac.samplingRate = uint32_t(s.rate);
        s.info.aac.isSbr = 0;
        s.ctrl.maxEsSize = SCE_AUDIODEC_AAC_MAX_ES_SIZE;
        type = SCE_AUDIODEC_TYPE_AAC;
    }
    const int r = sceAudiodecCreateDecoder(&s.ctrl,type);
    if (r < 0) {
        char text[96]; std::snprintf(text,sizeof(text),"sceAudiodecCreateDecoder failed 0x%08x",unsigned(r)); error=text; return false;
    }
    s.decoder_created = true;
    return true;
}

bool rewindDecoder(DbtbCompressedBgm& s) {
    if (!s.loop) return false;
    if (s.decoder_created) sceAudiodecClearContext(&s.ctrl);
    s.aac_index = 0;
    s.mp3_pos = s.mp3_start;
    s.decoded_frame = s.decoded_frames = 0;
    s.consecutive_errors = 0;
    return true;
}

// Small sequential read-ahead cache: original compressed AAC samples remain
// unmodified; a few tens of KiB are held in RAM rather than the entire song.
bool readIndexedSample(DbtbCompressedBgm& s, const AacSample& sample) {
    if (!s.indexed_file || sample.size>s.aac_es.size() ||
        sample.offset>s.physical_bytes || sample.size>s.physical_bytes-sample.offset) return false;
    if (sample.offset<s.read_base || sample.size>s.read_count ||
        size_t(sample.offset-s.read_base)>s.read_count-sample.size) {
        if (sample.offset>LONG_MAX ||
            std::fseek(s.indexed_file,long(sample.offset),SEEK_SET)!=0) return false;
        s.read_base=sample.offset;
        s.read_count=std::fread(s.read_ahead.data(),1,
            std::min(s.read_ahead.size(),s.physical_bytes-s.read_base),s.indexed_file);
        if (s.read_count<sample.size) return false;
    }
    std::memcpy(s.aac_es.data(),s.read_ahead.data()+size_t(sample.offset-s.read_base),sample.size);
    return true;
}

bool decodeChunk(DbtbCompressedBgm& s) {
    s.decoded_frame = s.decoded_frames = 0;
    for (;;) {
        if (s.codec == Codec::AacM4a) {
            if (s.aac_index >= s.aac_samples.size()) {
                if (!rewindDecoder(s)) return false;
            }
            const AacSample smp = s.aac_samples[s.aac_index++];
            if (s.indexed_file) {
                if (!readIndexedSample(s,smp)) return false;
                s.ctrl.pEs = s.aac_es.data();
            } else s.ctrl.pEs = s.bytes.data()+smp.offset;
            s.ctrl.maxEsSize = smp.size;
        } else {
            if (s.mp3_pos + 4 > s.bytes.size()) {
                if (!rewindDecoder(s)) return false;
            }
            const size_t remain = s.bytes.size()-s.mp3_pos;
            s.ctrl.pEs = s.bytes.data()+s.mp3_pos;
            s.ctrl.maxEsSize = uint32_t(std::min(remain,size_t(SCE_AUDIODEC_MP3_MAX_ES_SIZE)));
        }
        s.ctrl.inputEsSize = 0;
        s.ctrl.outputPcmSize = 0;
        s.ctrl.pPcm = s.decoded.data();
        const int r = sceAudiodecDecode(&s.ctrl);
        if (r >= 0 && s.ctrl.inputEsSize > 0 && s.ctrl.outputPcmSize > 0 &&
            s.ctrl.outputPcmSize <= s.ctrl.maxPcmSize &&
            (s.ctrl.outputPcmSize % (sizeof(int16_t)*size_t(s.channels))) == 0) {
            if (s.codec == Codec::Mp3) s.mp3_pos += s.ctrl.inputEsSize;
            s.decoded_frames = s.ctrl.outputPcmSize/(sizeof(int16_t)*size_t(s.channels));
            s.consecutive_errors = 0;
            return s.decoded_frames != 0;
        }
        if (s.codec == Codec::AacM4a) {
            return false;
        }
        if (++s.consecutive_errors > 64 || ++s.mp3_pos >= s.bytes.size()) return false;
    }
}

bool readFrame(DbtbCompressedBgm& s, int16_t& left, int16_t& right) {
    if (s.decoded_frame >= s.decoded_frames && !decodeChunk(s)) return false;
    const size_t p = s.decoded_frame++*size_t(s.channels);
    left = s.decoded[p];
    right = s.channels == 2 ? s.decoded[p+1] : left;
    return true;
}
} // namespace

DbtbCompressedBgm* dbtb_openCompressedBgm(const std::string& relative,
                                          float gain, bool loop,
                                          std::string& error) {
    error.clear();
    auto* s = new (std::nothrow) DbtbCompressedBgm();
    if (!s) { error = "compressed BGM state allocation failed"; return nullptr; }
    // M4A audio is indexed from its original file and decoded in 64 KiB
    // windows. Ogg/Vorbis and original MP3 paths are deliberately unchanged.
    std::string path;
    if (!dbtb_vfs().resolve(relative,path)) { delete s; error=dbtb_vfs().error(); return nullptr; }
    FILE* probe=std::fopen(path.c_str(),"rb");
    if (!probe) { delete s; error="compressed BGM open failed"; return nullptr; }
    uint8_t magic[8]{};
    const bool mp4=std::fread(magic,1,8,probe)==8 &&
                  std::memcmp(magic+4,"ftyp",4)==0;
    if (mp4) {
        if (std::fseek(probe,0,SEEK_END)!=0 || std::ftell(probe)<16) {
            std::fclose(probe);delete s;error="M4A source seek failed";return nullptr;
        }
        const long n=std::ftell(probe);
        s->indexed_file=probe;s->physical_bytes=size_t(n);
        std::vector<uint8_t> index;
        if (!readM4aIndex(probe,s->physical_bytes,index,error)) { delete s;return nullptr; }
        int ch=0,rate=0;
        if (!parseM4a(index,s->physical_bytes,ch,rate,s->aac_samples,error)) { delete s;return nullptr; }
        s->codec=Codec::AacM4a;s->channels=ch;s->rate=rate;
        std::vector<uint8_t>().swap(index);
        runtimeLog("Compressed BGM indexed: "+relative+" source_bytes="+
                   std::to_string(s->physical_bytes)+" sample_count="+
                   std::to_string(s->aac_samples.size()));
    } else {
        std::fclose(probe);
        if (!loadFile(relative,s->bytes,error)) { delete s; return nullptr; }
    }
    Mp3Header mh; size_t mp3_start = 0;
    if (s->indexed_file) {
        // AAC/M4A metadata was parsed without buffering the full media file.
    } else if (findMp3(s->bytes,mp3_start,mh)) {
        s->codec = Codec::Mp3;
        s->mp3_start = s->mp3_pos = mp3_start;
        s->mp3_version = mh.version;
        s->channels = int(mh.channels);
        s->rate = int(mh.rate);
    } else {
        int ch=0,rate=0;
        if (!parseM4a(s->bytes,s->bytes.size(),ch,rate,s->aac_samples,error)) { delete s; return nullptr; }
        s->codec = Codec::AacM4a;
        s->channels=ch; s->rate=rate;
    }
    if (s->rate <= 0 || (s->channels != 1 && s->channels != 2)) { delete s; error="compressed BGM format invalid"; return nullptr; }
    s->gain = std::max(0.0f,std::min(2.0f,gain));
    s->loop = loop;
    s->step = (uint64_t(uint32_t(s->rate)) << 32) / uint64_t(kOutputRate);
    if (!s->step || !createDecoder(*s,error)) { dbtb_closeCompressedBgm(s); return nullptr; }
    if (!readFrame(*s,s->current_l,s->current_r)) { error="compressed BGM first decode failed"; dbtb_closeCompressedBgm(s); return nullptr; }
    if (!readFrame(*s,s->next_l,s->next_r)) {
        s->next_l=s->current_l; s->next_r=s->current_r; s->eof_after_next=true;
    }
    s->primed=true;
    runtimeLog(std::string("Compressed BGM direct: ") + relative + " codec=" + dbtb_compressedBgmCodec(s) +
               " rate=" + std::to_string(s->rate) + " channels=" + std::to_string(s->channels) +
               " source_bytes=" + std::to_string(s->indexed_file?s->physical_bytes:s->bytes.size()));
    return s;
}

void dbtb_closeCompressedBgm(DbtbCompressedBgm* s) {
    if (!s) return;
    if (s->decoder_created) sceAudiodecDeleteDecoder(&s->ctrl);
    delete s;
}

void dbtb_mixCompressedBgm(DbtbCompressedBgm* s, int32_t& left, int32_t& right) {
    if (!s || !s->primed || !s->step) return;
    const uint32_t frac = uint32_t((s->phase >> 16) & 0xffffu);
    const uint32_t inv = 65536u-frac;
    const auto interp=[&](int32_t a,int32_t b){return (a*int32_t(inv)+b*int32_t(frac))>>16;};
    left += int32_t(interp(s->current_l,s->next_l)*s->gain);
    right += int32_t(interp(s->current_r,s->next_r)*s->gain);
    s->phase += s->step;
    const uint64_t one=uint64_t(1)<<32;
    while (s->phase >= one && s->primed) {
        s->phase -= one;
        if (s->eof_after_next) { s->primed=false; break; }
        s->current_l=s->next_l; s->current_r=s->next_r;
        if (!readFrame(*s,s->next_l,s->next_r)) {
            s->next_l=s->current_l; s->next_r=s->current_r; s->eof_after_next=true;
        }
    }
}

const char* dbtb_compressedBgmCodec(const DbtbCompressedBgm* s) {
    if (!s) return "none";
    return s->codec == Codec::Mp3 ? "mp3" : "aac-m4a";
}
