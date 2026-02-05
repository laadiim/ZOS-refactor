//
// Created by laadim on 28.01.26.
//

#include "../include/Bitmap.h"

#include <stdexcept>

// =====================================================
// Constructor
// =====================================================

/**
 * @brief Construct a bitmap with all bits cleared.
 */
Bitmap::Bitmap(const uint32_t bitCount)
    : size(bitCount),
      data((bitCount + 7) / 8, 0) {
}

// =====================================================
// Bit access
// =====================================================

/**
 * @brief Get allocation state of a bit.
 */
bool Bitmap::Get(const uint32_t index) const {
    const uint32_t byteIndex = index / 8;
    const uint32_t bitIndex  = index % 8;

    // Extract bit value (LSB-first)
    return (this->data[byteIndex] >> bitIndex) & 0x1;
}

/**
 * @brief Set or clear a bit.
 */
void Bitmap::Set(const uint32_t index, const bool value) {
    const uint32_t byteIndex = index / 8;
    const uint32_t bitIndex  = index % 8;

    if (value) {
        // Mark resource as allocated
        this->data[byteIndex] |= (1 << bitIndex);
    } else {
        // Mark resource as free
        this->data[byteIndex] &= ~(1 << bitIndex);
    }
}

// =====================================================
// Allocation helpers
// =====================================================

/**
 * @brief Find the first free (unset) bit.
 */
std::optional<uint32_t> Bitmap::FindFirstFree() const {
    for (uint32_t i = 0; i < this->size; ++i) {
        if (!this->Get(i)) {
            return i;
        }
    }
    return std::nullopt;
}

/**
 * @brief Count the number of free bits.
 */
uint32_t Bitmap::FreeCount() const {
    uint32_t count = 0;

    for (uint32_t i = 0; i < this->size; ++i) {
        if (!this->Get(i)) {
            ++count;
        }
    }

    return count;
}

// =====================================================
// Persistence
// =====================================================

/**
 * @brief Load bitmap state from raw bytes.
 */
Bitmap Bitmap::LoadFromBytes(std::vector<char> data,
                             const uint32_t bitCount) {
    Bitmap bitmap(bitCount);

    // Replace internal storage with loaded data
    bitmap.data = std::move(data);
    return bitmap;
}

/**
 * @brief Serialize bitmap to raw bytes.
 */
std::vector<char> Bitmap::SaveToBytes() const {
    return this->data;
}
