#pragma once
#include <chrono>
#include <optional>
#include <charconv>

using std::optional;

constexpr optional<int64_t> str2ll(const char* str, size_t len) {
    if (str == nullptr || len == 0) {
        return std::nullopt;
    }

    int64_t value;
    std::from_chars_result result = std::from_chars(str, str + len, value);

    if (result.ec == std::errc{} && result.ptr == str + len) {
        return value;
    }

    return std::nullopt;
}

constexpr uint64_t GetSecTimestamp() {
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
