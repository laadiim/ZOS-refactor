//
// Created by laadim on 28.01.26.
//

#pragma once
#include <cstdint>
#include <limits>
#include <string>

/**
 * @brief Parses a human-readable size string into bytes.
 *
 * Supported formats:
 *  - "123"   → 123 bytes
 *  - "123B"  → 123 bytes
 *  - "10KB"  → 10 * 1024 bytes
 *  - "5MB"   → 5 * 1024 * 1024 bytes
 *  - "1GB"   → 1 * 1024 * 1024 * 1024 bytes
 *
 * Suffix is case-insensitive.
 *
 * @param input Input string
 * @param outBytes Output value in bytes
 *
 * @return true if parsing succeeded, false otherwise
 */
inline bool ParseSize(const std::string& input, uint64_t& outBytes) {
    if (input.empty()) {
        return false;
    }

    // ---------------------------------
    // Parse numeric part
    // ---------------------------------
    size_t i = 0;
    while (i < input.size() && std::isdigit(input[i])) {
        ++i;
    }

    if (i == 0) {
        return false; // no numeric prefix
    }

    uint64_t value = 0;
    try {
        value = std::stoull(input.substr(0, i));
    } catch (...) {
        return false;
    }

    // ---------------------------------
    // Parse suffix
    // ---------------------------------
    std::string suffix = input.substr(i);
    for (char& c : suffix) {
        c = static_cast<char>(std::toupper(c));
    }

    uint64_t multiplier = 1;

    if (suffix.empty() || suffix == "B") {
        multiplier = 1;
    } else if (suffix == "KB") {
        multiplier = 1024ULL;
    } else if (suffix == "MB") {
        multiplier = 1024ULL * 1024ULL;
    } else if (suffix == "GB") {
        multiplier = 1024ULL * 1024ULL * 1024ULL;
    } else {
        return false; // unsupported unit
    }

    // ---------------------------------
    // Overflow protection
    // ---------------------------------
    if (value > std::numeric_limits<uint64_t>::max() / multiplier) {
        return false;
    }

    outBytes = value * multiplier;
    return true;
}