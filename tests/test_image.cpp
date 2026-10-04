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
            data.resize(24);
            CHECK(!decodePng(data,image,error) && image.pixels.empty());
        }
    }
    std::cout << "PNG PASS " << count << " original PNG entries decoded; truncated/invalid payload rejected\n";
}
