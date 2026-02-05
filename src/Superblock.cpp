//
// Created by laadim on 28.01.26.
//

#include "../include/Superblock.h"

#include "../helpers/IntParser.h"

#include <array>
#include <stdexcept>

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

std::array<char, Superblock::BYTE_SIZE> Superblock::toBytes() const {
    std::array<char, BYTE_SIZE> bytes{};
    size_t offset = 0;

    auto writeU32 = [&](uint32_t value) {
        auto data = IntParser::WriteUInt32(value);
        std::copy(data.begin(), data.end(), bytes.begin() + offset);
        offset += sizeof(uint32_t);
    };

    writeU32(magic);
    writeU32(blockSize);
    writeU32(totalBlocks);
    writeU32(totalInodes);
    writeU32(size);
    writeU32(inodeBitmapOffset);
    writeU32(blockBitmapOffset);
    writeU32(inodeTableOffset);
    writeU32(dataBlocksOffset);
    writeU32(rootNodeId);

    if (offset != BYTE_SIZE) {
        throw std::runtime_error("Superblock::toBytes size mismatch");
    }

    return bytes;
}

Superblock Superblock::fromBytes(std::array<char, BYTE_SIZE> data) {
    size_t offset = 0;

    auto readU32 = [&](uint32_t& out) {
        out = IntParser::ReadUInt32(
            std::vector<char>(data.begin() + offset,
                              data.begin() + offset + sizeof(uint32_t))
        );
        offset += sizeof(uint32_t);
    };

    Superblock sb{};

    readU32(sb.magic);
    readU32(sb.blockSize);
    readU32(sb.totalBlocks);
    readU32(sb.totalInodes);
    readU32(sb.size);
    readU32(sb.inodeBitmapOffset);
    readU32(sb.blockBitmapOffset);
    readU32(sb.inodeTableOffset);
    readU32(sb.dataBlocksOffset);
    readU32(sb.rootNodeId);

    if (offset != BYTE_SIZE) {
        throw std::runtime_error("Superblock::fromBytes size mismatch");
    }

    return sb;
}
