// Private normalized fixtures for the original JVM parser probe; never commit output.
#include "engine_resources.hpp"
#include <fstream>
#include <cstdio>
int main(int argc,char** argv) {
    if(argc!=4)return 2;
    GameVfs vfs(argv[1]);if(*argv[2]&&!vfs.selectMod(argv[2]))return 3;
    for(int i=0;i<13;++i) {
        char name[32];std::snprintf(name,sizeof(name),"char%02d.pac",i);
        std::string path,error;std::vector<uint8_t> bytes;
        if(!readEngineResource(vfs,name,bytes,path,error))return 4;
        std::ofstream out(std::string(argv[3])+"/"+name,std::ios::binary);
        out.write(reinterpret_cast<const char*>(bytes.data()),bytes.size());if(!out)return 5;
    }
}
