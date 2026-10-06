#pragma once

#include <cstddef>
#include <cstdint>

enum class PacEncoding {
    Auto,
    Original,
    Community14,
    Community14Spanish,
    Community14Invasion
};

struct CommunityPacProfile {
    PacEncoding encoding;
    const char* name;
    uint16_t count_xor;
    uint32_t offset_xor;
    uint32_t size_xor;
    uint16_t image_width_xor;
    uint16_t image_height_xor;
    uint16_t table_count_xor;
    uint32_t table_position_xor;
    uint16_t table_width_xor;
    uint16_t table_height_xor;
    uint32_t wav_size_xor;
    // Encoded directory stores BE32(type_key ^ entry_index). Zero means the
    // current audited corpus never exposed that type for this profile.
    uint32_t type_act;
    uint32_t type_bin;
    uint32_t type_cnv;
    uint32_t type_dac;
    uint32_t type_rgba;
    uint32_t type_spr;
    uint32_t type_wav;
};

inline const CommunityPacProfile* communityProfiles(size_t& count) {
    static const CommunityPacProfile profiles[] = {
        {
            PacEncoding::Community14, "community14-a210795b",
            0xa732u, 0x3b681c6bu, 0x02d6d26eu,
            0xf34du, 0x93f9u,
            0x8722u, 0x8f7fc2cau, 0x5addu, 0xb9e8u,
            0xa732u,
            0x98fa9490u, 0x4a4aece2u, 0x4396461cu, 0x41b6196du,
            0x42412565u, 0x469b0775u, 0x873be70du
        },
        {
            PacEncoding::Community14Spanish, "community14-es-d594affc",
            0xe6aau, 0x31874c24u, 0x790e6bafu,
            0x29cdu, 0x42acu,
            0x2ea3u, 0x07db0921u, 0x941fu, 0x126fu,
            0xe6aau,
            0u, 0x00fc517eu, 0xedb419c8u, 0xd49fade7u,
            0x5ee0f896u, 0x03c296fdu, 0x5bad42a6u
        },
        {
            PacEncoding::Community14Invasion, "community14-invasion-05aa0c5e",
            0x842fu, 0x71573adbu, 0x33ac6051u,
            0xa42cu, 0xed15u,
            0x68a3u, 0x122e64cbu, 0xb1d9u, 0x7c00u,
            0x842fu,
            0u, 0xaebfc3a0u, 0x877e379fu, 0x8125d853u,
            0xe7c20ecbu, 0x403d58e7u, 0x153ccb39u
        }
    };
    count = sizeof(profiles) / sizeof(profiles[0]);
    return profiles;
}

inline const CommunityPacProfile* communityProfile(PacEncoding encoding) {
    size_t count = 0;
    const CommunityPacProfile* profiles = communityProfiles(count);
    for (size_t i = 0; i < count; ++i)
        if (profiles[i].encoding == encoding) return &profiles[i];
    return nullptr;
}

inline bool isCommunityEncoding(PacEncoding encoding) {
    return communityProfile(encoding) != nullptr;
}

inline const char* communityEncodingName(PacEncoding encoding) {
    if (encoding == PacEncoding::Original) return "original";
    const CommunityPacProfile* profile = communityProfile(encoding);
    return profile ? profile->name : "auto/unknown";
}

inline const char* communityType(const CommunityPacProfile& profile, uint32_t key) {
    if (profile.type_act && key == profile.type_act) return "act";
    if (key == profile.type_bin) return "bin";
    if (key == profile.type_cnv) return "cnv";
    if (key == profile.type_dac) return "dac";
    if (key == profile.type_rgba) return "rgba";
    if (key == profile.type_spr) return "spr";
    if (key == profile.type_wav) return "wav";
    return nullptr;
}

inline uint16_t communityReadLe16(const uint8_t* p) {
    return uint16_t(p[0]) | (uint16_t(p[1]) << 8);
}
inline uint32_t communityReadLe32(const uint8_t* p) {
    return uint32_t(p[0]) | (uint32_t(p[1]) << 8) |
           (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}
inline uint32_t communityReadBe32(const uint8_t* p) {
    return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) |
           (uint32_t(p[2]) << 8) | uint32_t(p[3]);
}

// Detect only audited protected PACs. Every directory extent must be valid and
// at least one known runtime type must be present. Ordinary PACs return Original;
// malformed/unknown protected data remains distinguishable by its later bounds
// checks instead of being guessed as another profile.
inline PacEncoding detectCommunityEncoding(const uint8_t* data, size_t size) {
    if (!data || size < 18) return PacEncoding::Original;
    const uint16_t raw_count = communityReadLe16(data);
    if (!(raw_count & 0x8000u)) return PacEncoding::Original;
    size_t profile_count = 0;
    const CommunityPacProfile* profiles = communityProfiles(profile_count);
    PacEncoding match = PacEncoding::Original;
    unsigned matches = 0;
    for (size_t p = 0; p < profile_count; ++p) {
        const CommunityPacProfile& profile = profiles[p];
        const size_t count = raw_count ^ profile.count_xor;
        const uint64_t base = 2ull + uint64_t(count) * 16ull;
        if (!count || base > size) continue;
        bool known = false, valid = true;
        for (size_t i = 0; i < count; ++i) {
            const uint8_t* row = data + 2 + i * 16;
            const uint32_t offset = communityReadLe32(row) ^ profile.offset_xor ^ uint32_t(i);
            const uint32_t bytes = communityReadLe32(row + 4) ^ profile.size_xor ^ uint32_t(i);
            if (base + uint64_t(offset) > size || uint64_t(bytes) > size - base - uint64_t(offset)) {
                valid = false; break;
            }
            const uint32_t key = communityReadBe32(row + 8) ^ uint32_t(i);
            known |= communityType(profile, key) != nullptr;
        }
        if (valid && known) { match = profile.encoding; ++matches; }
    }
    return matches == 1 ? match : PacEncoding::Original;
}
