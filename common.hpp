#pragma once

#include <string>

#define logErr(m, ...)                                                         \
    fprintf(stderr, "[%s(%s:%d)]Error: " m "\n", __func__, __FILE__, __LINE__, \
            ##__VA_ARGS__)
#define logInfo(m, ...) fprintf(stdout, m "\n", ##__VA_ARGS__)
#define ASSERT(cond, err)                                                      \
    if ((cond))                                                                \
        throw err;

struct Config {
    std::string serverName;
    std::string serverPort;
    uint txDepth = 512;
    size_t blockSize = 65536;
};
static Config config;