// Exact native resource imports; GL uploads are mocked, no renderer claim.
#define DBTB_SAVE_BASENAME "save-controls-test.bin"
#include "../tools/aot/engine/native/resources.cpp"
#include <cassert>
#include <fstream>
#include <iostream>

int main(int argc,char** argv) {
    assert(argc>1);
    for(int profile=1;profile<argc;profile++) {
        std::vector<uint8_t> stable(kSaveSize,7);stable[4]=2;
        const std::string stablePath=std::string(argv[profile])+"/profiles/Sample/save.bin";
        {std::ofstream f(stablePath,std::ios::binary);f.write(reinterpret_cast<const char*>(stable.data()),stable.size());assert(f);}
        dbtb_setControlMode(0);assert(dbtb_initResources(argv[profile],"Sample"));
        assert(save_path!=stablePath && save_cache.size()==kSaveSize);
        std::vector<uint8_t> isolated;assert(readFile(save_path,isolated));
        char effect[]="effect";int size=dbtb_resource(effect);assert(size>0);
        auto cached=pending_resource;std::vector<uint8_t> original(size);dbtb_copyResource(original.data(),size);
        assert(original==cached->bytes);
        dbtb_setControlMode(2);int stream=dbtb_openResourceStream(effect,0);assert(stream>0);
        const auto& view=resource_streams.at(stream);assert(view.resource==cached && view.hidden.size()==50);
        std::vector<uint8_t> hidden(size);
        // Includes reads split between the two bytes of each image field.
        for(int p=0;p<size;p+=7) {int n=std::min(7,size-p);assert(dbtb_readResourceStream(stream,p,hidden.data()+p,n)==n);}
        for(size_t i=0;i<original.size();i++) {
            bool target=std::binary_search(view.hidden.begin(),view.hidden.end(),i);
            assert(hidden[i]==(target?255:original[i]));
        }
        assert(cached->bytes==original); // No mutable shared-cache graphics.
        dbtb_setControlMode(1);int visible=dbtb_openResourceStream(effect,0);assert(visible>0);
        std::vector<uint8_t> restored(size);assert(dbtb_readResourceStream(visible,0,restored.data(),size)==size);assert(restored==original);
        // An open stream snapshots visibility even after another mode/cache read.
        assert(dbtb_readResourceStream(stream,0,restored.data(),size)==size && restored==hidden);
        dbtb_closeResourceStream(visible);dbtb_closeResourceStream(stream);
        dbtb_setControlMode(2);assert(dbtb_resource(effect)==size);dbtb_copyResource(restored.data(),size);assert(restored==hidden);
        assert(dbtb_resourceFiltered(effect,16)>0 && pending_hidden.empty());
        pending_resource.reset();
        // Save overlays preserve touch mode on disk and every progress byte.
        save_cache.resize(kSaveSize);for(size_t i=0;i<kSaveSize;i++)save_cache[i]=uint8_t(i*37);save_cache[4]=2;
        save_cache_exists=save_cache_known=true;auto previous=save_cache;
        char save[]="save.bin";assert(dbtb_readSave(save)==int(kSaveSize));std::vector<uint8_t> runtimeSave(kSaveSize);dbtb_copyResource(runtimeSave.data(),kSaveSize);
        assert(runtimeSave[4]==1);runtimeSave[4]=2;assert(runtimeSave==previous);
        runtimeSave[4]=1;runtimeSave[30]=123;
        assert(dbtb_writeSave(save,runtimeSave.data(),kSaveSize,0,1)==1 && save_cache[4]==2 && save_cache[30]==123);
        std::vector<uint8_t> disk;assert(readFile(save_path,disk) && disk==save_cache);
        dbtb_setControlMode(0);assert(dbtb_readSave(save)==int(kSaveSize));dbtb_copyResource(runtimeSave.data(),kSaveSize);assert(runtimeSave==disk);
        assert(readFile(stablePath,isolated) && isolated==stable);
        // Truncated/aliased/unknown DAC must fail without a partial overlay.
        std::vector<size_t> rejected;std::string why;auto corrupt=original;corrupt.resize(20);
        assert(!VitaPadVisibility::build(corrupt,rejected,why) && rejected.empty());
        // Aliasing a foreign action onto a pad record must reject all fields.
        corrupt=original;size_t pacBase=2+VitaPadVisibility::u16(corrupt.data())*16;
        size_t dac=0;for(size_t i=0;i<VitaPadVisibility::u16(corrupt.data());i++) {
            auto* entry=corrupt.data()+2+i*16;if(!memcmp(entry+8,"dac",3))dac=pacBase+VitaPadVisibility::u32(entry);
        }
        assert(dac);size_t index=dac+VitaPadVisibility::u16(corrupt.data()+dac+4);
        corrupt[index]=corrupt[index+200*2];corrupt[index+1]=corrupt[index+200*2+1];
        assert(!VitaPadVisibility::build(corrupt,rejected,why) && rejected.empty());
        std::cout << "CONTROLS RESOURCE PASS " << argv[profile] << " (50 sparse bytes; immutable cache; mode snapshot; fragmented reads; save preference/progress)\n";
    }
    return 0;
}
