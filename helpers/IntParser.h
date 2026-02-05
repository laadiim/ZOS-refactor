//
// Created by laadim on 26.01.26.
//

#pragma once
#include <array>
#include <cstdint>
#include <vector>

/**
 * @brief Utility class for serializing and deserializing integers.
 *
 * Provides helper functions to convert fixed-width unsigned integers
 * to and from byte buffers using a defined byte order.
 *
 * All values are encoded and decoded using little-endian format.
 */
class IntParser {
public:
    /**
     * @brief Parse a 32-bit unsigned integer from a byte buffer.
     *
     * Interprets the buffer as a little-endian encoded uint32_t.
     *
     * @param data Byte buffer (must contain exactly 4 bytes).
     * @return Parsed 32-bit unsigned integer.
     *
     * @throws std::runtime_error If the buffer size is incorrect.
     */
    static uint32_t ReadUInt32(const std::vector<char>& data);

    /**
     * @brief Parse a 64-bit unsigned integer from a byte buffer.
     *
     * Interprets the buffer as a little-endian encoded uint64_t.
     *
     * @param data Byte buffer (must contain exactly 8 bytes).
     * @return Parsed 64-bit unsigned integer.
     *
     * @throws std::runtime_error If the buffer size is incorrect.
     */
    static uint64_t ReadUInt64(const std::vector<char>& data);

    /**
     * @brief Serialize a 32-bit unsigned integer into a byte buffer.
     *
     * Encodes the value using little-endian byte order.
     *
     * @param number Value to serialize.
     * @return Byte buffer containing the encoded value.
     */
    static std::vector<char> WriteUInt32(const uint32_t& number);

    /**
     * @brief Serialize a 64-bit unsigned integer into a byte buffer.
     *
     * Encodes the value using little-endian byte order.
     *
     * @param number Value to serialize.
     * @return Byte buffer containing the encoded value.
     */
    static std::vector<char> WriteUInt64(const uint64_t& number);
};
