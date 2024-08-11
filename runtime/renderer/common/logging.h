#include <cstdio>
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>
// #define LOGI printf
// #define LOGW printf
// #define LOGE printf
// #define LOGD printf

#define LOGI(...) spdlog::info(__VA_ARGS__);
#define LOGW(...) spdlog::warn(__VA_ARGS__);
#define LOGE(...) printf
#define LOGD(...) spdlog::debug(__VA_ARGS__);