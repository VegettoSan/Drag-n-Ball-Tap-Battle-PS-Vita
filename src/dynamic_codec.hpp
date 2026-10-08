#pragma once
#include "community_profiles.hpp"
#include <cctype>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

// Strict, data-only codec metadata. The APK is NOT parsed/executed by Vita.
// Exactly one bounded JSON object with a fixed schema; unknown fields,
// duplicates, path strings, floats, negative numbers and trailing junk reject.
inline bool parseDynamicCodecJson(const std::vector<uint8_t>& bytes,
                                 CommunityPacProfile& out, std::string& error) {
    error.clear();
    if (bytes.empty() || bytes.size() > 4096) { error = "codec metadata size outside budget"; return false; }
    const std::string doc(bytes.begin(), bytes.end());
    size_t pos = 0;
    const auto space = [&]() {
        while (pos < doc.size() && (doc[pos]==' '||doc[pos]=='\n'||doc[pos]=='\r'||doc[pos]=='\t')) ++pos;
    };
    const auto take = [&](char x) { space(); if (pos<doc.size()&&doc[pos]==x) { ++pos; return true; } return false; };
    const auto stringValue = [&](std::string& value) {
        space();
        if (pos >= doc.size() || doc[pos++] != '"') return false;
        const size_t start=pos;
        while (pos < doc.size() && doc[pos]!='"') {
            const unsigned char c=static_cast<unsigned char>(doc[pos]);
            if (c<0x20 || c>0x7e || c=='\\' || pos-start>80) return false;
            ++pos;
        }
        if (pos>=doc.size())return false;
        value.assign(doc,start,pos-start);++pos;
        return true;
    };
    std::map<std::string,uint32_t> numbers;
    std::string generator;
    if (!take('{')) { error="codec metadata must be a JSON object"; return false; }
    while (true) {
        space();
        if (take('}')) break;
        std::string key;
        if (!stringValue(key) || !take(':')) { error="invalid codec JSON key"; return false; }
        if (key=="generator") {
            if (!generator.empty() || !stringValue(generator)) { error="invalid codec generator"; return false; }
        } else {
            uint32_t value=0;
            space();const size_t start=pos;
            while (pos<doc.size() && doc[pos]>='0' && doc[pos]<='9') {
                const unsigned digit=unsigned(doc[pos++]-'0');
                if (value>(UINT32_MAX-digit)/10u) { error="codec integer overflow"; return false; }
                value=value*10u+digit;
            }
            if (pos==start || numbers.count(key)) { error="duplicate or invalid codec field"; return false; }
            numbers.emplace(key,value);
        }
        space();
        if (take('}')) break;
        if (!take(',')) { error="missing codec JSON delimiter"; return false; }
    }
    space();
    if (pos!=doc.size() || generator!="dragontap-private-v1" || numbers.size()!=18 || !numbers.count("format") || numbers["format"]!=1u) {
        error="codec schema/version mismatch";return false;
    }
    const char* required[]={
        "count_xor","offset_xor","size_xor",
        "image_width_xor","image_height_xor","table_count_xor",
        "table_position_xor","table_width_xor","table_height_xor",
        "wav_size_xor","type_act","type_bin","type_cnv","type_dac",
        "type_rgba","type_spr","type_wav"
    };
    for (const char* key : required) if (!numbers.count(key)) { error=std::string("codec missing ")+key;return false; }
    for (const auto& entry : numbers) {
        if (entry.first=="format") continue;
        bool allowed=false;
        for (const char* key:required) if(entry.first==key){allowed=true;break;}
        if (!allowed) { error="unrecognized codec field";return false; }
    }
    const auto u16 = [&](const char* key) { return numbers[key] <= 65535u; };
    if (!u16("count_xor") || !u16("image_width_xor") || !u16("image_height_xor") ||
        !u16("table_count_xor") || !u16("table_width_xor") || !u16("table_height_xor") ||
        numbers["wav_size_xor"]!=numbers["count_xor"]) {
        error="invalid codec width or WAV contract";return false;
    }
    const char* types[]={"type_act","type_bin","type_cnv","type_dac","type_rgba","type_spr","type_wav"};
    for (size_t i=0;i<7;++i)for(size_t j=i+1;j<7;++j)
        if (numbers[types[i]]==numbers[types[j]]) { error="duplicate protected type";return false; }
    out=CommunityPacProfile{
        PacEncoding::Community14Dynamic, "dragontap-private-v1",
        uint16_t(numbers["count_xor"]),numbers["offset_xor"],numbers["size_xor"],
        uint16_t(numbers["image_width_xor"]),uint16_t(numbers["image_height_xor"]),
        uint16_t(numbers["table_count_xor"]),numbers["table_position_xor"],
        uint16_t(numbers["table_width_xor"]),uint16_t(numbers["table_height_xor"]),
        numbers["wav_size_xor"],
        numbers["type_act"],numbers["type_bin"],numbers["type_cnv"],numbers["type_dac"],
        numbers["type_rgba"],numbers["type_spr"],numbers["type_wav"]
    };
    return true;
}
