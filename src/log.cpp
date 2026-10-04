#include "log.hpp"
#include <cstdio>
#include <ctime>

void runtimeLog(const std::string& line) {
    FILE* fp = std::fopen("ux0:data/DBTapBattle/logs/runtime.log", "ab");
    if (!fp) { std::fprintf(stderr, "DBTB log unavailable: %s\n", line.c_str()); return; }
    const std::time_t now = std::time(nullptr);
    const std::tm* stamp = std::localtime(&now);
    char date[32] = "time unavailable";
    if (stamp) std::strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", stamp);
    std::fprintf(fp, "[%s] %s\n", date, line.c_str());
    std::fclose(fp);
}
