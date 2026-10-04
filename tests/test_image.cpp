#include "image.hpp"
#include "pac.hpp"
#include <iostream>
#define CHECK(x) do { if (!(x)) { std::cerr << "FAIL image " << __LINE__ << '\n'; return 1; } } while (0)
int main(int argc, char** argv) {
    RgbaImage image; std::string error;
    CHECK(!decodePng({1,2,3},image,error));
    CHECK(image.pixels.empty() && !error.empty());
    size_t count=0;
    for (int i=1;i<argc;++i) {
        PacFile pac; CHECK(pac.open(argv[i]));
        for (size_t n=0;n<pac.entries().size();++n) {
            if (pac.typeString(n)!="png") continue;
            std::vector<uint8_t> data;CHECK(pac.readEntry(n,data));
            CHECK(decodePng(data,image,error));
            CHECK(image.pixels.size()==static_cast<size_t>(image.width)*image.height*4);
            ++count;
            auto oversized = data;
            // Valid IHDR CRC with 4096x4096 dimensions: reject decoded 64-MiB budget
            // before allocating pixels, even though compressed entry is small.
            for (int k=0;k<8;++k) oversized[16+k]=0;
            oversized[18]=oversized[22]=16;
            uint32_t crc=0xffffffffu;
            for (int k=12;k<29;++k) {
                crc ^= oversized[k];
                for(int bit=0;bit<8;++bit) crc=(crc>>1) ^ ((crc&1)?0xedb88320u:0);
            }
            crc ^= 0xffffffffu;
            for(int k=0;k<4;++k) oversized[29+k]=static_cast<uint8_t>(crc>>(24-k*8));
            CHECK(!decodePng(oversized,image,error) && image.pixels.empty());
            CHECK(error.find("budget")!=std::string::npos);
            data.resize(24);
            CHECK(!decodePng(data,image,error) && image.pixels.empty());
        }
    }
    std::cout << "PNG PASS " << count << " original PNG entries decoded; truncated/invalid payload rejected\n";
}
