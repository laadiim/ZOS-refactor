//
// Created by laadim on 26.01.26.
//

#include "IntParser.h"

#include <stdexcept>

/// Number of bits in a byte
constexpr uint32_t BYTESIZE = 8;

/**
 * @brief Parse a 32-bit unsigned integer from a byte buffer.
 */
uint32_t IntParser::ReadUInt32(const std::vector<char>& data) {
    // Validate buffer size
    if (data.size() != sizeof(uint32_t)) {
        throw std::runtime_error("Incorrect data size");
    }

    // Assemble little-endian integer byte-by-byte
    return  static_cast<uint32_t>(static_cast<uint8_t>(data[0])) |
            static_cast<uint32_t>(static_cast<uint8_t>(data[1])) << (BYTESIZE * 1) |
            static_cast<uint32_t>(static_cast<uint8_t>(data[2])) << (BYTESIZE * 2) |
            static_cast<uint32_t>(static_cast<uint8_t>(data[3])) << (BYTESIZE * 3);
}

/**
 * @brief Parse a 64-bit unsigned integer from a byte buffer.
 */
uint64_t IntParser::ReadUInt64(const std::vector<char>& data) {
    // Validate buffer size
    if (data.size() != sizeof(uint64_t)) {
        throw std::runtime_error("Incorrect data size");
    }

    // Assemble little-endian integer byte-by-byte
    return  static_cast<uint64_t>(static_cast<uint8_t>(data[0])) |
            static_cast<uint64_t>(static_cast<uint8_t>(data[1])) << (BYTESIZE * 1) |
            static_cast<uint64_t>(static_cast<uint8_t>(data[2])) << (BYTESIZE * 2) |
            static_cast<uint64_t>(static_cast<uint8_t>(data[3])) << (BYTESIZE * 3) |
            static_cast<uint64_t>(static_cast<uint8_t>(data[4])) << (BYTESIZE * 4) |
            static_cast<uint64_t>(static_cast<uint8_t>(data[5])) << (BYTESIZE * 5) |
            static_cast<uint64_t>(static_cast<uint8_t>(data[6])) << (BYTESIZE * 6) |
            static_cast<uint64_t>(static_cast<uint8_t>(data[7])) << (BYTESIZE * 7);
}

/**
 * @brief Serialize a 32-bit unsigned integer into a byte buffer.
 */
std::vector<char> IntParser::WriteUInt32(const uint32_t& number) {
    std::vector<char> data(sizeof(uint32_t));

    // Write bytes least-significant first (little-endian)
    data[0] = static_cast<char>((number >> (BYTESIZE * 0)) & 0xFF);
    data[1] = static_cast<char>((number >> (BYTESIZE * 1)) & 0xFF);
    data[2] = static_cast<char>((number >> (BYTESIZE * 2)) & 0xFF);
    data[3] = static_cast<char>((number >> (BYTESIZE * 3)) & 0xFF);

    return data;
}

/**
 * @brief Serialize a 64-bit unsigned integer into a byte buffer.
 */
std::vector<char> IntParser::WriteUInt64(const uint64_t& number) {
    std::vector<char> data(sizeof(uint64_t));

    // Write bytes least-significant first (little-endian)
    data[0] = static_cast<char>((number >> (BYTESIZE * 0)) & 0xFF);
    data[1] = static_cast<char>((number >> (BYTESIZE * 1)) & 0xFF);
    data[2] = static_cast<char>((number >> (BYTESIZE * 2)) & 0xFF);
    data[3] = static_cast<char>((number >> (BYTESIZE * 3)) & 0xFF);
    data[4] = static_cast<char>((number >> (BYTESIZE * 4)) & 0xFF);
    data[5] = static_cast<char>((number >> (BYTESIZE * 5)) & 0xFF);
    data[6] = static_cast<char>((number >> (BYTESIZE * 6)) & 0xFF);
    data[7] = static_cast<char>((number >> (BYTESIZE * 7)) & 0xFF);

    return data;
}
