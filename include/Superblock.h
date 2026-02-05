//
// Created by laadim on 28.01.26.
//

#pragma once
#include <cstdint>
#include <vector>

/**
 * @brief Filesystem magic number.
 *
 * Used to detect whether a filesystem image is valid and formatted.
 * Stored in the superblock and checked during filesystem initialization.
 */
constexpr uint32_t FILESYSTEM_MAGIC = 0xDEADBEEF;

/**
 * @class Superblock
 * @brief Global filesystem metadata.
 *
 * The superblock is stored at the very beginning of the filesystem image
 * (byte offset 0). It describes:
 *
 *  - filesystem geometry (block size, counts)
 *  - allocation state (free blocks / inodes)
 *  - on-disk layout (offsets of all major structures)
 *
 * The superblock is required to correctly interpret all other data
 * stored in the filesystem image.
 */
class Superblock {
public:
    // ========================
    // Identification
    // ========================

    /**
     * @brief Filesystem magic number.
     *
     * Used to verify that the image file contains a valid filesystem
     * of the expected type.
     */
    uint32_t magic;

    // ========================
    // Geometry
    // ========================

    /**
     * @brief Size of a single data block in bytes.
     *
     * All block allocations and offsets are calculated using this value.
     */
    uint32_t blockSize;

    /**
     * @brief Total number of data blocks in the filesystem.
     */
    uint32_t totalBlocks;

    /**
     * @brief Total number of inodes in the filesystem.
     */
    uint32_t totalInodes;

    /**
     * @brief Total size of the filesystem image in bytes.
     */
    uint32_t size;

    // ========================
    // Layout (byte offsets)
    // ========================

    /**
     * @brief Byte offset of the inode bitmap.
     *
     * Each bit corresponds to one inode (free / used).
     */
    uint32_t inodeBitmapOffset;

    /**
     * @brief Byte offset of the data block bitmap.
     *
     * Each bit corresponds to one data block (free / used).
     */
    uint32_t blockBitmapOffset;

    /**
     * @brief Byte offset of the inode table.
     *
     * The inode table stores serialized inode structures.
     */
    uint32_t inodeTableOffset;

    /**
     * @brief Byte offset of the first data block.
     */
    uint32_t dataBlocksOffset;

    /**
     * @brief Inode ID of the root directory.
     *
     * Typically inode 0.
     */
    uint32_t rootNodeId;

    // ========================
    // Serialization
    // ========================

    /**
     * @brief Serialized size of the superblock in bytes.
     *
     * This value must remain constant to allow correct deserialization.
     */
    static constexpr std::size_t BYTE_SIZE = 40;

    /*
     * offset | item
     * =================
     *      0 | magic number
     *      4 | block size
     *      8 | total blocks
     *     12 | total inodes
     *     16 | filesystem size
     *     20 | inode bitmap offset
     *     24 | block bitmap offset
     *     28 | inode table offset
     *     32 | data blocks offset
     *     36 | root node id
     * =================
     * TOTAL = 40 bytes
     */

    /**
     * @brief Serializes the superblock into a byte array.
     *
     * The serialized layout is fixed and independent of compiler padding.
     * This method is used when writing the superblock to disk.
     *
     * @return Byte vector containing the serialized superblock.
     */
    [[nodiscard]] std::array<char, BYTE_SIZE> toBytes() const;

    /**
     * @brief Deserializes a superblock from raw bytes.
     *
     * Reconstructs a Superblock instance from data previously written
     * by toBytes().
     *
     * @param data Raw byte data read from the filesystem image.
     * @return Reconstructed Superblock instance.
     */
    static Superblock fromBytes(std::array<char, BYTE_SIZE> data);
};