#pragma once

#include <array>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <istream>
#include <ostream>

/**
 * @class Bitmap
 * @brief Bitmap structure for resource allocation.
 *
 * Bitmap is used to track allocation state of fixed-size resources,
 * such as:
 *  - inodes
 *  - data blocks
 *
 * Each bit represents one resource:
 *  - bit = 0 → free
 *  - bit = 1 → allocated
 *
 * The bitmap is stored in a packed byte representation
 * and can be serialized to / deserialized from disk.
 */
class Bitmap {
public:
    /**
     * @brief Construct a bitmap with a given number of bits.
     *
     * All bits are initialized to 0 (free).
     *
     * @param bitCount Number of tracked resources.
     */
    explicit Bitmap(uint32_t bitCount);

    // =====================================================
    // Bit access
    // =====================================================

    /**
     * @brief Get allocation state of a bit.
     *
     * @param index Bit index.
     * @return True if bit is set (allocated), false if free.
     *
     * @note No bounds checking is performed.
     */
    [[nodiscard]] bool Get(uint32_t index) const;

    /**
     * @brief Set or clear a bit.
     *
     * @param index Bit index.
     * @param value True = allocated, false = free.
     *
     * @note No bounds checking is performed.
     */
    void Set(uint32_t index, bool value);

    // =====================================================
    // Allocation helpers
    // =====================================================

    /**
     * @brief Find the first free (unset) bit.
     *
     * @return Index of the first free bit, or std::nullopt if full.
     */
    [[nodiscard]] std::optional<uint32_t> FindFirstFree() const;

    /**
     * @brief Count the number of free bits.
     *
     * @return Number of free (unset) bits.
     */
    [[nodiscard]] uint32_t FreeCount() const;

    // =====================================================
    // Persistence
    // =====================================================

    /**
     * @brief Load a bitmap from raw byte data.
     *
     * @param data Raw bitmap bytes read from disk.
     * @param bitCount Number of bits represented.
     * @return Reconstructed Bitmap instance.
     */
    static Bitmap LoadFromBytes(std::vector<char> data, uint32_t bitCount);

    /**
     * @brief Serialize bitmap to raw byte data.
     *
     * @return Packed bitmap bytes.
     */
    [[nodiscard]] std::vector<char> SaveToBytes() const;

    // =====================================================
    // Internal state
    // =====================================================

    /**
     * @brief Number of bits managed by the bitmap.
     */
    uint32_t size;

    /**
     * @brief Packed bit data.
     *
     * Bits are packed LSB-first into bytes.
     */
    std::vector<char> data;
};
