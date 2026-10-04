#include "uchar.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"UTF16 FAIL line %d\n",__LINE__); return 1; } } while(0)
int main(void) {
    const char* text = "A\xc3\xb1\xe6\x97\xa5\xf0\x9f\x90\x89";
    const char16_t chars[] = {0x41,0xf1,0x65e5,0xd83d,0xdc09};
    mbstate_t state = {0}; size_t used=0;
    for (size_t i=0;i<5;++i) {
        char16_t c; size_t n=mbrtoc16(&c,text+used,strlen(text)-used,&state);
        CHECK(c==chars[i] && n!=(size_t)-1 && n!=(size_t)-2);
        if(n!=(size_t)-3) used+=n;
    }
    CHECK(used==strlen(text));
    char16_t c=1; CHECK(mbrtoc16(&c,"",1,&state)==0 && c==0);
    char result[32];used=0;
    for(size_t i=0;i<5;++i) {
        size_t n=c16rtomb(result+used,chars[i],&state); CHECK(n!=(size_t)-1);used+=n;
    }
    result[used]=0; CHECK(strcmp(text,result)==0);
    memset(&state,0,sizeof state);
    CHECK(mbrtoc16(&c,"\xf0\x9f",2,&state)==(size_t)-2);
    CHECK(mbrtoc16(&c,"\x90\x89",2,&state)==2 && c==0xd83d);
    CHECK(mbrtoc16(&c,"",0,&state)==(size_t)-3 && c==0xdc09);
    const char* invalid[]={"\xc0\xaf","\xed\xa0\x80","\xf4\x90\x80\x80","\xe0\x80\x80","\xe2\x28\xa1"};
    for(size_t i=0;i<5;++i) {
        memset(&state,0,sizeof state);errno=0;
        CHECK(mbrtoc16(&c,invalid[i],strlen(invalid[i]),&state)==(size_t)-1 && errno==EILSEQ);
        CHECK(mbrtoc16(&c,"A",1,&state)==1 && c=='A');
    }
    CHECK(c16rtomb(result,0xdc00,&state)==(size_t)-1);
    CHECK(c16rtomb(result,0xd800,&state)==0);
    CHECK(c16rtomb(result,'A',&state)==(size_t)-1);
    CHECK(c16rtomb(NULL,0,&state)==1);
    CHECK(c16rtomb(result,0,&state)==1 && result[0]==0);
    puts("AOT UTF16 PASS");return 0;
}
