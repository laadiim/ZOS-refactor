//
// Created by laadim on 28.01.26.
//

#include "../include/Filesystem.h"

#include <algorithm>
#include <iostream>
#include <sstream>

#include "../helpers/FileIOExceptions.h"
#include "../helpers/FilesystemExceptions.h"
#include "../helpers/IntParser.h"
#include "../helpers/StringHelpers.h"

Filesystem::Filesystem(const std::string& imagePath)
    : FileIO(std::make_unique<FileIOHandler>()),
      INodeBitmap(0),
      BlockBitmap(0),
      imagePath(imagePath) {

    this->FileIO->OpenFile(
        this->imagePath,
        FileIOHandler::FileModes::READ_WRITE
    );

    // Attempt to read superblock
    auto sbData = this->FileIO->ReadBytes(
        0,
        Superblock::BYTE_SIZE
    );

    if (sbData.size() != Superblock::BYTE_SIZE) {
        return;
    }

    std::array<char, Superblock::BYTE_SIZE> sbBytes{};
    std::copy(sbData.begin(), sbData.end(), sbBytes.begin());

    this->superblock = Superblock::fromBytes(sbBytes);

    // Invalid magic → not formatted
    if (this->superblock.magic != FILESYSTEM_MAGIC) {
        return;
    }

    // Load inode bitmap
    const uint32_t inodeBitmapBytes =
        (this->superblock.totalInodes + 7) / 8;

    this->INodeBitmap = Bitmap::LoadFromBytes(
        this->FileIO->ReadBytes(
            this->superblock.inodeBitmapOffset,
            inodeBitmapBytes
        ),
        this->superblock.totalInodes
    );

    // Load block bitmap
    const uint32_t blockBitmapBytes =
        (this->superblock.totalBlocks + 7) / 8;

    this->BlockBitmap = Bitmap::LoadFromBytes(
        this->FileIO->ReadBytes(
            this->superblock.blockBitmapOffset,
            blockBitmapBytes
        ),
        this->superblock.totalBlocks
    );

    // Load root inode
    this->currentNode =
        this->readINode(this->superblock.rootNodeId);

    this->formated = true;
}

Filesystem::~Filesystem() {
    if (!this->formated) {
        this->FileIO->CloseFile();
        return;
    }

    // Persist superblock
    auto sb = this->superblock.toBytes();
    this->FileIO->WriteBytes(
        0,
        std::vector<char>(sb.begin(), sb.end())
    );

    // Persist bitmaps
    this->FileIO->WriteBytes(
        this->superblock.inodeBitmapOffset,
        this->INodeBitmap.SaveToBytes()
    );

    this->FileIO->WriteBytes(
        this->superblock.blockBitmapOffset,
        this->BlockBitmap.SaveToBytes()
    );

    this->FileIO->Flush();
    this->FileIO->CloseFile();
}


void Filesystem::Format(const uint32_t bytes) {
    // Resize backing image
    if (this->FileIO->Resize(bytes) != bytes) {
        throw CouldNotResizeImageException(
            "Could not resize image"
        );
    }

    constexpr uint32_t BLOCK_SIZE = 1024;

    uint32_t blocks = bytes / BLOCK_SIZE;
    uint32_t inodes = 0;

    // Compute geometry that fits
    while (blocks > 0) {
        constexpr uint32_t BLOCKS_PER_INODE = 4;
        inodes = blocks / BLOCKS_PER_INODE;

        uint32_t metadata =
            Superblock::BYTE_SIZE +
            ((inodes + 7) / 8) +
            ((blocks + 7) / 8) +
            inodes * INode::BYTES;

        if (metadata + blocks * BLOCK_SIZE <= bytes) {
            break;
        }
        --blocks;
    }

    if (blocks == 0 || inodes == 0) {
        throw InvalidFilesystemSizeException(
            "Filesystem too small"
        );
    }

    // Initialize superblock
    this->superblock = Superblock{};
    this->superblock.magic = FILESYSTEM_MAGIC;
    this->superblock.blockSize = BLOCK_SIZE;
    this->superblock.totalBlocks = blocks;
    this->superblock.totalInodes = inodes;
    this->superblock.size = bytes;

    this->superblock.inodeBitmapOffset =
        Superblock::BYTE_SIZE;

    this->superblock.blockBitmapOffset =
        this->superblock.inodeBitmapOffset +
        (inodes + 7) / 8;

    this->superblock.inodeTableOffset =
        this->superblock.blockBitmapOffset +
        (blocks + 7) / 8;

    this->superblock.dataBlocksOffset =
        this->superblock.inodeTableOffset +
        inodes * INode::BYTES;

    // Initialize bitmaps
    this->INodeBitmap = Bitmap(inodes);
    this->BlockBitmap = Bitmap(blocks);

    // Allocate root inode
    auto root = this->AllocateNode(true);
    if (!root) {
        throw CouldNotAllocateNodeException(
            "Could not allocate root"
        );
    }

    this->currentNode = *root;
    this->superblock.rootNodeId = root->getId();

    // Add "." and ".."
    this->AddChild(*root, ".", root->getId());
    this->AddChild(*root, "..", root->getId());

    // Persist initial metadata
    auto sb = this->superblock.toBytes();
    this->FileIO->WriteBytes(
        0,
        std::vector<char>(sb.begin(), sb.end())
    );

    this->FileIO->WriteBytes(
        this->superblock.inodeBitmapOffset,
        this->INodeBitmap.SaveToBytes()
    );

    this->FileIO->WriteBytes(
        this->superblock.blockBitmapOffset,
        this->BlockBitmap.SaveToBytes()
    );

    this->writeINode(this->currentNode);
    this->formated = true;
}

bool Filesystem::Formated() const {
    return this->formated;
}

INode Filesystem::readINode(const uint32_t id) const {
    auto data = this->FileIO->ReadBytes(
        this->superblock.inodeTableOffset +
        id * INode::BYTES,
        INode::BYTES
    );

    if (data.size() != INode::BYTES) {
        throw InvalidINodeSizeException(
            "Invalid inode size"
        );
    }

    return INode::FromBytes(data);
}

void Filesystem::writeINode(const INode& node) const {
    const uint64_t offset =
        this->superblock.inodeTableOffset +
        node.getId() * INode::BYTES;

    this->FileIO->WriteBytes(offset, node.ToBytes());
}

std::optional<INode> Filesystem::AllocateNode(const bool isDir) {
    const auto id = INodeBitmap.FindFirstFree();
    if (!id) return std::nullopt;

    INodeBitmap.Set(*id, true);
    INode node(*id, isDir);

    if (isDir) {
        auto b = AllocateBlock();
        if (b == std::nullopt) {
            INodeBitmap.Set(*id, false);
            return std::nullopt;
        }
        this->FileIO->WriteBytes(this->superblock.dataBlocksOffset + *b * this->superblock.blockSize, std::vector<char>(this->superblock.blockSize, 0xFF));
        AttachBlock(node, *b);
    }
    this->writeINode(node);

    return node;
}

void Filesystem::FreeNode(const INode& node) {
    this->INodeBitmap.Set(node.getId(), false);
    for (auto b : this->GetAllBlockIds(node)) {
        this->FreeBlock(b);
    }
    const std::vector<char> toWrite(INode::BYTES, 0);
    this->FileIO->WriteBytes(this->superblock.inodeTableOffset + INode::BYTES * node.getId(), toWrite);
}

std::optional<uint32_t> Filesystem::AllocateBlock() {
    const auto block = this->BlockBitmap.FindFirstFree();
    if (block != std::nullopt) {
        this->BlockBitmap.Set(*block, true);
    }
    return block;
}

void Filesystem::FreeBlock(const uint32_t block) {
    this->BlockBitmap.Set(block, false);
    this->FileIO->WriteBytes(this->superblock.dataBlocksOffset + block * this->superblock.blockSize, std::vector<char>(this->superblock.blockSize, 0));
}

void Filesystem::AddChild(INode &node, std::string name, uint32_t childNode) {
    if (!node.isDir()) {
        throw NotADirectoryException("Target node is not a directory");
    }
    // construct data for writing
    std::vector<char> toWrite(name.begin(), name.end());
    while (toWrite.size() < 12) {
        toWrite.push_back('\0');
    }
    auto childBytes = IntParser::WriteUInt32(childNode);
    for (auto b : childBytes) {
        toWrite.push_back(b);
    }
    constexpr uint32_t ENTRYSIZE = 12 + sizeof(uint32_t);

    // attempt using direct blocks
    for (auto block : node.getDirectLinks()) {
        if (block == INode::UNUSED_LINK) {
            auto tmp = this->AllocateBlock();
            if (tmp == std::nullopt) {
                throw CouldNotAllocateBlockException("Could not allocate block");
            }

            this->FileIO->WriteBytes(
                    this->superblock.dataBlocksOffset +
                    *tmp * this->superblock.blockSize,
                    std::vector<char>(this->superblock.blockSize, 0xFF)
                );

            node.addDirectLink(*tmp);
            block = *tmp;
            writeINode(node);

        }

        auto children = this->ReadBlockAsSubdirectories(block);

        if (children.size() < this->superblock.blockSize / ENTRYSIZE) {
            const uint32_t offset =
                this->superblock.dataBlocksOffset +
                block * this->superblock.blockSize +
                children.size() * ENTRYSIZE;  // ✅ correct index domain

            this->FileIO->WriteBytes(offset, toWrite);

            writeINode(node);

            return;
        }
    }

    {
        // allocate indirect block if necessary
        if (node.getFirstLevelIndirectLink() == INode::UNUSED_LINK) {
            const auto indirectBlock = this->AllocateBlock();
            if (indirectBlock == std::nullopt) {
                throw CouldNotAllocateBlockException("Could not allocate block");
            }

            this->FileIO->WriteBytes(
                    this->superblock.dataBlocksOffset +
                    *indirectBlock * this->superblock.blockSize,
                    std::vector<char>(this->superblock.blockSize, 0xFF)
                );

            node.addFirstLevelIndirectLink(*indirectBlock);

            writeINode(node);
        }

        auto indirects = this->ReadBlockAsBlockIds(node.getFirstLevelIndirectLink());

        // attempt to use first level indirect block
        for (auto block : indirects) {
            auto children = this->ReadBlockAsSubdirectories(block);
            if (children.size() < this->superblock.blockSize / ENTRYSIZE) {
                const uint32_t offset =
                    this->superblock.dataBlocksOffset +
                    block * this->superblock.blockSize +
                    children.size() * ENTRYSIZE;

                this->FileIO->WriteBytes(offset, toWrite);

                writeINode(node);
                return;
            }
        }

        if (indirects.size() < this->superblock.blockSize / sizeof(uint32_t)) {
            auto newBlock = this->AllocateBlock();
            if (newBlock == std::nullopt) {
                throw CouldNotAllocateBlockException("Could not allocate block");
            }

            this->FileIO->WriteBytes(
                    this->superblock.dataBlocksOffset +
                    *newBlock * this->superblock.blockSize,
                    std::vector<char>(this->superblock.blockSize, 0xFF)
                );

            this->FileIO->WriteBytes(
                    this->superblock.dataBlocksOffset +
                    node.getFirstLevelIndirectLink() * this->superblock.blockSize +
                    indirects.size() * sizeof(uint32_t),
                    IntParser::WriteUInt32(*newBlock)
                );

            const uint32_t offset =
                this->superblock.dataBlocksOffset +
                *newBlock * this->superblock.blockSize;

            this->FileIO->WriteBytes(offset, toWrite);

            writeINode(node);
            return;
        }
    }

    {
        // use second level indirect block

        if (node.getSecondLevelIndirectLink() == INode::UNUSED_LINK) {
            const auto indirectBlock = this->AllocateBlock();
            if (indirectBlock == std::nullopt) {
                throw CouldNotAllocateBlockException("Could not allocate block");
            }

            this->FileIO->WriteBytes(
                    this->superblock.dataBlocksOffset +
                    *indirectBlock * this->superblock.blockSize,
                    std::vector<char>(this->superblock.blockSize, 0xFF)
                );

            node.addSecondLevelIndirectLink(*indirectBlock);

            writeINode(node);
        }

        auto indirects = this->ReadBlockAsBlockIds(node.getSecondLevelIndirectLink());
        for (auto block : indirects) {
            auto indirects2 = this->ReadBlockAsBlockIds(block);

            for (auto block2 : indirects2) {
                auto children = this->ReadBlockAsSubdirectories(block2);
                if (children.size() < this->superblock.blockSize / ENTRYSIZE) {
                    const uint32_t offset =
                        this->superblock.dataBlocksOffset +
                        block2 * this->superblock.blockSize +
                        children.size() * ENTRYSIZE;

                    this->FileIO->WriteBytes(offset, toWrite);

                    writeINode(node);
                    return;
                }
            }

            if (indirects2.size() < this->superblock.blockSize / sizeof(uint32_t)) {
                auto newBlock = this->AllocateBlock();
                if (newBlock == std::nullopt) {
                    throw CouldNotAllocateBlockException("Could not allocate block");
                }

                this->FileIO->WriteBytes(
                        this->superblock.dataBlocksOffset +
                        *newBlock * this->superblock.blockSize,
                        std::vector<char>(this->superblock.blockSize, 0xFF)
                    );

                this->FileIO->WriteBytes(
                        this->superblock.dataBlocksOffset +
                        block * this->superblock.blockSize +
                        indirects2.size() * sizeof(uint32_t),
                        IntParser::WriteUInt32(*newBlock)
                    );

                const uint32_t offset =
                    this->superblock.dataBlocksOffset +
                    *newBlock * this->superblock.blockSize;

                this->FileIO->WriteBytes(offset, toWrite);

                writeINode(node);
                return;
            }
        }

        if (indirects.size() < this->superblock.blockSize / sizeof(uint32_t)) {
            auto newBlock = this->AllocateBlock();
            if (newBlock == std::nullopt) {
                throw CouldNotAllocateBlockException("Could not allocate block");
            }

            this->FileIO->WriteBytes(
                    this->superblock.dataBlocksOffset +
                    *newBlock * this->superblock.blockSize,
                    std::vector<char>(this->superblock.blockSize, 0xFF)
                );

            this->FileIO->WriteBytes(
                    this->superblock.dataBlocksOffset +
                    node.getSecondLevelIndirectLink() * this->superblock.blockSize +
                    indirects.size() * sizeof(uint32_t),
                    IntParser::WriteUInt32(*newBlock)
                );

            auto newBlock2 = this->AllocateBlock();
            if (newBlock2 == std::nullopt) {
                throw CouldNotAllocateBlockException("Could not allocate block");
            }

            this->FileIO->WriteBytes(
                    this->superblock.dataBlocksOffset +
                    *newBlock2 * this->superblock.blockSize,
                    std::vector<char>(this->superblock.blockSize, 0xFF)
                );

            this->FileIO->WriteBytes(
                    this->superblock.dataBlocksOffset +
                    *newBlock * this->superblock.blockSize,
                    IntParser::WriteUInt32(*newBlock2)
                );

            const uint32_t offset =
                this->superblock.dataBlocksOffset +
                *newBlock2 * this->superblock.blockSize;

            this->FileIO->WriteBytes(offset, toWrite);

            writeINode(node);
            return;
        }
    }

    throw FileTooLargeException("Directory is full");
}

void Filesystem::AttachBlock(INode& node, const uint32_t block) {
    std::vector<char> entry = IntParser::WriteUInt32(block);
    constexpr uint32_t ENTRYSIZE = sizeof(uint32_t);

    const uint32_t IDS_PER_BLOCK =
        superblock.blockSize / ENTRYSIZE;

    // =========================
    // 1) DIRECT BLOCKS
    // =========================
    for (auto b : node.getDirectLinks()) {
        if (b == INode::UNUSED_LINK) {
            node.addDirectLink(block);
            writeINode(node);
            return;
        }
    }

    // =========================
    // 2) SINGLE INDIRECT
    // =========================
    if (node.getFirstLevelIndirectLink() == INode::UNUSED_LINK) {
        auto ind = AllocateBlock();
        if (!ind) throw CouldNotAllocateBlockException("Could not allocate block");

        // init pointer table
        FileIO->WriteBytes(
            superblock.dataBlocksOffset + *ind * superblock.blockSize,
            std::vector<char>(superblock.blockSize, 0xFF)
        );

        node.addFirstLevelIndirectLink(*ind);
        writeINode(node);
    }

    {
        uint32_t ind = node.getFirstLevelIndirectLink();
        auto ids = ReadBlockAsBlockIds(ind);

        if (ids.size() < IDS_PER_BLOCK) {
            uint64_t offset =
                superblock.dataBlocksOffset +
                ind * superblock.blockSize +
                ids.size() * ENTRYSIZE;

            FileIO->WriteBytes(offset, entry);
            return;
        }
    }

    // =========================
    // 3) DOUBLE INDIRECT
    // =========================
    if (node.getSecondLevelIndirectLink() == INode::UNUSED_LINK) {
        auto ind2 = AllocateBlock();
        if (!ind2) throw CouldNotAllocateBlockException("Could not allocate block");

        FileIO->WriteBytes(
            superblock.dataBlocksOffset + *ind2 * superblock.blockSize,
            std::vector<char>(superblock.blockSize, 0xFF)
        );

        node.addSecondLevelIndirectLink(*ind2);
        writeINode(node);
    }

    uint32_t ind2 = node.getSecondLevelIndirectLink();
    auto firstLevelPtrs = ReadBlockAsBlockIds(ind2);

    // try existing second-level blocks
    for (uint32_t ptr : firstLevelPtrs) {
        auto ids = ReadBlockAsBlockIds(ptr);

        if (ids.size() < IDS_PER_BLOCK) {
            uint64_t offset =
                superblock.dataBlocksOffset +
                ptr * superblock.blockSize +
                ids.size() * ENTRYSIZE;

            FileIO->WriteBytes(offset, entry);
            return;
        }
    }

    // need a NEW second-level block
    if (firstLevelPtrs.size() < IDS_PER_BLOCK) {
        auto newPtr = AllocateBlock();
        if (!newPtr) throw CouldNotAllocateBlockException("Could not allocate block");

        FileIO->WriteBytes(
            superblock.dataBlocksOffset + *newPtr * superblock.blockSize,
            std::vector<char>(superblock.blockSize, 0xFF)
        );

        // link it into double-indirect table
        uint64_t ptrOffset =
            superblock.dataBlocksOffset +
            ind2 * superblock.blockSize +
            firstLevelPtrs.size() * ENTRYSIZE;

        FileIO->WriteBytes(ptrOffset, IntParser::WriteUInt32(*newPtr));

        // write first data entry
        uint64_t dataOffset =
            superblock.dataBlocksOffset +
            *newPtr * superblock.blockSize;

        FileIO->WriteBytes(dataOffset, entry);
        return;
    }

    throw FileTooLargeException("No room for new blocks");
}

std::vector<ChildNodeNameIdPair> Filesystem::ReadBlockAsSubdirectories(const uint32_t block) const {
    const auto data = FileIO->ReadBytes(
        superblock.dataBlocksOffset +
        superblock.blockSize * block,
        superblock.blockSize
    );

    if (data.size() != superblock.blockSize) {
        throw InvalidBlockSizeException("Could not read block " + std::to_string(block));
    }

    std::vector<ChildNodeNameIdPair> childNodes;

    constexpr size_t ENTRY_SIZE = sizeof(uint32_t) + 12;
    size_t offset = 0;

    while (offset + ENTRY_SIZE <= data.size()) {
        // ---- read fixed-length name ----
        std::string name(
            data.begin() + offset,
            data.begin() + offset + 12
        );
        offset += 12;

        auto nul = name.find('\0');
        if (nul != std::string::npos) {
            name.resize(nul);
        }

        // ---- read inode id ----
        uint32_t inodeId = IntParser::ReadUInt32(
            std::vector<char>(
                data.begin() + offset,
                data.begin() + offset + sizeof(uint32_t)
            )
        );
        offset += sizeof(uint32_t);

        // Unused entry → end
        if (inodeId == INode::UNUSED_LINK) {
            break;
        }

        childNodes.push_back({ name, inodeId });
    }

    return childNodes;
}

std::vector<uint32_t> Filesystem::ReadBlockAsBlockIds(const uint32_t block) const {
    const auto data = FileIO->ReadBytes(
        superblock.dataBlocksOffset +
        superblock.blockSize * block,
        superblock.blockSize
    );

    if (data.size() != superblock.blockSize) {
        throw InvalidBlockSizeException("Could not read block" + std::to_string(block));
    }

    std::vector<uint32_t> children;
    size_t offset = 0;
    while (offset + sizeof(uint32_t) <= data.size()) {
        auto number = IntParser::ReadUInt32(std::vector<char>(data.begin() + offset, data.begin() + offset + sizeof(uint32_t)));
        if (number == INode::UNUSED_LINK) {
            break;
        }
        children.push_back(number);
        offset += sizeof(uint32_t);
    }
    return children;
}

std::vector<ChildNodeNameIdPair> Filesystem::GetChildren(const INode &node) const {
    if (!node.isDir()) {
        throw NotADirectoryException("Target not a directory");
    }
    std::vector<ChildNodeNameIdPair> children;

    for (auto block : node.getDirectLinks()) {
        if (block == INode::UNUSED_LINK) {
            return children;
        }
        for (auto child : this->ReadBlockAsSubdirectories(block)) {
            children.push_back(child);
        }
    }

    const auto indirect = node.getFirstLevelIndirectLink();
    if (indirect == INode::UNUSED_LINK) {
        return children;
    }
    for (auto childBlock : this->ReadBlockAsBlockIds(indirect)) {
        if (childBlock == INode::UNUSED_LINK) {
            return children;
        }
        for (auto child : this->ReadBlockAsSubdirectories(childBlock)) {
            children.push_back(child);
        }
    }

    const auto indirect2 = node.getSecondLevelIndirectLink();
    if (indirect2 == INode::UNUSED_LINK) {
        return children;
    }
    for (auto childBlock : this->ReadBlockAsBlockIds(indirect2)) {
        if (childBlock == INode::UNUSED_LINK) {
            return children;
        }
        for (auto childChildBlock : this->ReadBlockAsBlockIds(childBlock)) {
            if (childChildBlock == INode::UNUSED_LINK) {
                return children;
            }
            for (auto child : this->ReadBlockAsSubdirectories(childChildBlock)) {
                children.push_back(child);
            }
        }
    }
    return children;
}

void Filesystem::RemoveChild(const INode& node, const uint32_t childNode) const {
    if (!node.isDir()) {
        throw NotADirectoryException("Target not a directory");
    }

    constexpr uint32_t ENTRYSIZE = 12 + sizeof(uint32_t);

    struct EntryLoc {
        uint32_t block;
        uint32_t index;
    };

    std::optional<EntryLoc> target;
    std::optional<EntryLoc> last;

    auto scanBlock = [&](uint32_t blockId) {
        auto entries = ReadBlockAsSubdirectories(blockId);
        for (uint32_t i = 0; i < entries.size(); ++i) {
            if (entries[i].id == childNode) {
                target = { blockId, i };
            }
            last = { blockId, i };
        }
    };

    // =========================
    // Scan direct blocks
    // =========================
    for (auto block : node.getDirectLinks()) {
        if (block == INode::UNUSED_LINK) break;
        scanBlock(block);
    }

    // =========================
    // Scan single indirect
    // =========================
    if (node.getFirstLevelIndirectLink() != INode::UNUSED_LINK) {
        for (auto block : ReadBlockAsBlockIds(node.getFirstLevelIndirectLink())) {
            scanBlock(block);
        }
    }

    // =========================
    // Scan double indirect
    // =========================
    if (node.getSecondLevelIndirectLink() != INode::UNUSED_LINK) {
        for (auto ptr : ReadBlockAsBlockIds(node.getSecondLevelIndirectLink())) {
            for (auto block : ReadBlockAsBlockIds(ptr)) {
                scanBlock(block);
            }
        }
    }

    if (!target) {
        throw ChildNotFoundException("Child " + std::to_string(childNode) + "not found");
    }

    // =========================
    // If target == last → just clear
    // =========================
    if (target->block == last->block &&
        target->index == last->index) {

        uint64_t offset =
            superblock.dataBlocksOffset +
            last->block * superblock.blockSize +
            last->index * ENTRYSIZE;

        FileIO->WriteBytes(offset, std::vector<char>(ENTRYSIZE, 0xFF));
        return;
    }

    // =========================
    // Move last entry into target
    // =========================
    uint64_t lastOffset =
        superblock.dataBlocksOffset +
        last->block * superblock.blockSize +
        last->index * ENTRYSIZE;

    auto lastData = FileIO->ReadBytes(lastOffset, ENTRYSIZE);
    if (lastData.size() != ENTRYSIZE) {
        throw std::runtime_error("Failed to read last entry");
    }

    uint64_t targetOffset =
        superblock.dataBlocksOffset +
        target->block * superblock.blockSize +
        target->index * ENTRYSIZE;

    // overwrite removed entry
    FileIO->WriteBytes(targetOffset, lastData);

    // clear last slot
    FileIO->WriteBytes(lastOffset, std::vector<char>(ENTRYSIZE, 0xFF));
}

bool Filesystem::RemoveFromBlockIdTable(const uint32_t tableBlock, const uint32_t value) {
    constexpr uint32_t ENTRYSIZE = sizeof(uint32_t);

    auto ids = ReadBlockAsBlockIds(tableBlock);
    if (ids.empty()) return false;

    int target = -1;
    for (int i = 0; i < static_cast<int>(ids.size()); ++i) {
        if (ids[i] == value) {
            target = i;
            break;
        }
    }
    if (target == -1) return false;

    int last = static_cast<int>(ids.size()) - 1;

    uint64_t targetOffset =
        superblock.dataBlocksOffset +
        tableBlock * superblock.blockSize +
        target * ENTRYSIZE;

    uint64_t lastOffset =
        superblock.dataBlocksOffset +
        tableBlock * superblock.blockSize +
        last * ENTRYSIZE;

    if (target != last) {
        auto lastBytes = FileIO->ReadBytes(lastOffset, ENTRYSIZE);
        FileIO->WriteBytes(targetOffset, lastBytes);
    }

    // clear last entry
    FileIO->WriteBytes(lastOffset, std::vector<char>(ENTRYSIZE, 0xFF));
    return true;
}

void Filesystem::DeattachBlock(INode& node, const uint32_t block) {
    // =========================
    // 1) Direct blocks
    // =========================
    for (auto b : node.getDirectLinks()) {
        if (b == block) {
            node.removeDirectLink(block);
            FreeBlock(block);
            writeINode(node);
            return;
        }
    }

    // =========================
    // 2) Single indirect
    // =========================
    if (node.getFirstLevelIndirectLink() != INode::UNUSED_LINK) {
        uint32_t ind = node.getFirstLevelIndirectLink();

        if (RemoveFromBlockIdTable(ind, block)) {
            FreeBlock(block);

            // if indirect table is now empty, free it
            if (ReadBlockAsBlockIds(ind).empty()) {
                FreeBlock(ind);
                node.removeFirstLevelIndirectLink();
            }

            writeINode(node);
            return;
        }
    }

    // =========================
    // 3) Double indirect
    // =========================
    if (node.getSecondLevelIndirectLink() != INode::UNUSED_LINK) {
        uint32_t ind2 = node.getSecondLevelIndirectLink();

        auto ptrs = ReadBlockAsBlockIds(ind2);
        for (uint32_t ptr : ptrs) {
            if (RemoveFromBlockIdTable(ptr, block)) {
                FreeBlock(block);

                // if second-level table empty, free it
                if (ReadBlockAsBlockIds(ptr).empty()) {
                    RemoveFromBlockIdTable(ind2, ptr);
                    FreeBlock(ptr);
                }

                // if double-indirect table empty, free it
                if (ReadBlockAsBlockIds(ind2).empty()) {
                    FreeBlock(ind2);
                    node.removeSecondLevelIndirectLink();
                }

                writeINode(node);
                return;
            }
        }
    }

    throw BlockNotAttachedException("Block not attached to inode");
}

std::optional<uint32_t> Filesystem::FindChildId(const INode &dir, std::string name) const {
    const auto children = this->GetChildren(dir);
    for (auto child : children) {
        if (child.name == name) {
            return child.id;
        }
    }
    return std::nullopt;
}

bool Filesystem::ExistsChild(const INode &dir, const std::string& name) const {
    return this->FindChildId(dir, name) != std::nullopt;
}

std::vector<uint32_t> Filesystem::GetAllBlockIds(const INode &node) const {
    std::vector<uint32_t> ids;

    for (auto b : node.getDirectLinks()) {
        if (b != INode::UNUSED_LINK) {
            ids.push_back(b);
        }
    }
    auto first = node.getFirstLevelIndirectLink();
    if (first != INode::UNUSED_LINK) {
        ids.push_back(first);
        for (auto b : this->ReadBlockAsBlockIds(first)) {
            if (b != INode::UNUSED_LINK) {
                ids.push_back(b);
            }
        }
    }

    if (node.getSecondLevelIndirectLink() != INode::UNUSED_LINK) {
        ids.push_back(node.getSecondLevelIndirectLink());
        for (auto b : this->ReadBlockAsBlockIds(node.getSecondLevelIndirectLink())) {
            if (b != INode::UNUSED_LINK) {
                ids.push_back(b);
                for (auto b2 : this->ReadBlockAsBlockIds(b)) {
                    if (b2 != INode::UNUSED_LINK) {
                        ids.push_back(b2);
                    }
                }
            }
        }
    }
    return ids;
}

INode Filesystem::ResolvePath(const std::string& path) const {
    if (path.empty()) {
        throw EmptyPathException("Empty path");
    }

    INode node =
        (path[0] == '/')
            ? this->readINode(this->superblock.rootNodeId)
            : this->currentNode;

    for (const auto& part : SplitPath(path)) {
        if (part == ".") {
            continue;
        }

        if (part == "..") {
            auto parent = this->FindChildId(node, "..");
            if (!parent) {
                throw NoParentDirectoryException(
                    "No parent directory"
                );
            }
            node = this->readINode(*parent);
            continue;
        }

        if (!node.isDir()) {
            throw NotADirectoryException(
                "Path component is not a directory"
            );
        }

        auto next = this->FindChildId(node, part);
        if (!next) {
            throw PathNotFoundException(
                "Path not found: " + part
            );
        }

        node = this->readINode(*next);
    }

    return node;
}

INode Filesystem::ResolveParent(const std::string& path) const {
    if (path.empty()) {
        throw std::runtime_error("Empty path");
    }

    INode node = (path[0] == '/')
        ? readINode(superblock.rootNodeId)
        : currentNode;

    auto parts = SplitPath(path);
    parts.pop_back();
    for (const auto& part : parts) {
        if (part == ".") {
            continue;
        }

        if (part == "..") {
            auto parent = FindChildId(node, "..");
            if (!parent) {
                throw std::runtime_error("No parent directory");
            }
            node = readINode(*parent);
            continue;
        }

        if (!node.isDir()) {
            throw std::runtime_error("Path component is not a directory");
        }

        auto next = FindChildId(node, part);
        if (!next) {
            throw std::runtime_error("Path not found: " + part);
        }

        node = readINode(*next);
    }

    return node;
}

void Filesystem::CreateDirectory(const std::string& path) {
    if (path.empty()) {
        throw EmptyPathException("Empty path");
    }

    INode parent = this->ResolveParent(path);
    auto name = SplitPath(path).back();

    auto newNode = this->AllocateNode(true);
    if (!newNode) {
        throw CouldNotAllocateNodeException(
            "Could not allocate directory inode"
        );
    }

    try {
        this->AddChild(parent, name, newNode->getId());
        this->AddChild(*newNode, ".", newNode->getId());
        this->AddChild(*newNode, "..", parent.getId());
    }
    catch (...) {
        this->FreeNode(*newNode);
        throw;
    }
}

void Filesystem::RemoveDirectory(const std::string& path) {
    if (path.empty()) {
        throw EmptyPathException("Empty path");
    }

    if (path == "/") {
        throw std::runtime_error("Cannot remove root directory");
    }

    INode parent = this->ResolveParent(path);
    auto parts = SplitPath(path);
    const std::string& name = parts.back();

    auto id = this->FindChildId(parent, name);
    if (!id) {
        throw PathNotFoundException("Directory not found");
    }

    if (*id == this->currentNode.getId()) {
        throw PathNotFoundException("Cannot remove current directory");
    }

    INode dir = this->readINode(*id);
    if (!dir.isDir()) {
        throw NotADirectoryException("Target is not a directory");
    }

    if (this->GetChildren(dir).size() > 2) {
        throw std::runtime_error("Directory not empty");
    }

    this->RemoveChild(parent, dir.getId());
    this->writeINode(parent);
    this->FreeNode(dir);
}


void Filesystem::WriteFile(const std::string& srcPath, std::vector<char> data) {
    if (srcPath.empty()) {
        throw EmptyPathException("Empty path");
    }

    // =========================
    // Resolve parent directory
    // =========================
    INode parent = ResolveParent(srcPath);
    if (!parent.isDir()) {
        throw NotADirectoryException("Parent is not a directory");
    }

    auto parts = SplitPath(srcPath);
    const std::string& filename = parts.back();

    if (filename.empty()) {
        throw PathNotFoundException("Invalid file name");
    }

    // =========================
    // Find or create inode
    // =========================
    std::optional<uint32_t> fileId = FindChildId(parent, filename);
    INode file;

    if (fileId) {
        file = readINode(*fileId);

        if (file.isDir()) {
            throw NotADirectoryException("Cannot write to a directory");
        }

        // =========================
        // Free existing blocks
        // =========================
        for (uint32_t block : file.getDirectLinks()) {
            if (block != INode::UNUSED_LINK) {
                FreeBlock(block);
            }
        }

        if (file.getFirstLevelIndirectLink() != INode::UNUSED_LINK) {
            for (uint32_t block : ReadBlockAsBlockIds(file.getFirstLevelIndirectLink())) {
                FreeBlock(block);
            }
            FreeBlock(file.getFirstLevelIndirectLink());
            file.removeFirstLevelIndirectLink();
        }

        if (file.getSecondLevelIndirectLink() != INode::UNUSED_LINK) {
            for (uint32_t ptr : ReadBlockAsBlockIds(file.getSecondLevelIndirectLink())) {
                for (uint32_t block : ReadBlockAsBlockIds(ptr)) {
                    FreeBlock(block);
                }
                FreeBlock(ptr);
            }
            FreeBlock(file.getSecondLevelIndirectLink());
            file.removeSecondLevelIndirectLink();
        }

        file.clearDirectLinks();
        file.removeSize(file.getSize());
    } else {
        auto newNode = AllocateNode(false);
        if (!newNode) {
            throw CouldNotAllocateNodeException("Could not allocate file inode");
        }

        file = *newNode;
        AddChild(parent, filename, file.getId());
        writeINode(parent);
    }

    // =========================
    // Write file data
    // =========================
    size_t written = 0;
    const size_t total = data.size();
    const size_t blockSize = superblock.blockSize;

    while (written < total) {
        auto blockOpt = AllocateBlock();
        if (!blockOpt) {
            throw CouldNotAllocateBlockException("No free blocks for file data");
        }

        uint32_t block = *blockOpt;

        size_t chunk = std::min(blockSize, total - written);

        FileIO->WriteBytes(
            superblock.dataBlocksOffset + block * blockSize,
            std::vector<char>(data.begin() + written, data.begin() + written + chunk)
        );

        AttachBlock(file, block);
        written += chunk;
    }

    file.addSize(total);
    writeINode(file);
}

std::vector<char> Filesystem::ReadFile(const std::string& srcPath) {
    if (srcPath.empty()) {
        throw EmptyPathException("Empty path");
    }

    INode file = ResolvePath(srcPath);

    if (file.isDir()) {
        throw NotADirectoryException("Cannot read a directory");
    }

    std::vector<char> result;
    result.reserve(file.getSize());

    const uint32_t blockSize = superblock.blockSize;
    size_t remaining = file.getSize();

    auto readBlock = [&](uint32_t blockId) {
        if (remaining == 0) return;

        size_t toRead = std::min(static_cast<size_t>(blockSize), remaining);

        auto data = FileIO->ReadBytes(
            superblock.dataBlocksOffset + blockId * blockSize,
            toRead
        );

        if (data.size() != toRead) {
            throw FileReadException("Failed to read file block");
        }

        result.insert(result.end(), data.begin(), data.end());
        remaining -= toRead;
    };

    // =========================
    // Direct blocks
    // =========================
    for (uint32_t block : file.getDirectLinks()) {
        if (block == INode::UNUSED_LINK || remaining == 0) break;
        readBlock(block);
    }

    // =========================
    // Single indirect
    // =========================
    if (remaining > 0 && file.getFirstLevelIndirectLink() != INode::UNUSED_LINK) {
        for (uint32_t block : ReadBlockAsBlockIds(file.getFirstLevelIndirectLink())) {
            if (remaining == 0) break;
            readBlock(block);
        }
    }

    // =========================
    // Double indirect
    // =========================
    if (remaining > 0 && file.getSecondLevelIndirectLink() != INode::UNUSED_LINK) {
        for (uint32_t ptr : ReadBlockAsBlockIds(file.getSecondLevelIndirectLink())) {
            for (uint32_t block : ReadBlockAsBlockIds(ptr)) {
                if (remaining == 0) break;
                readBlock(block);
            }
            if (remaining == 0) break;
        }
    }

    return result;
}

void Filesystem::CopyFile(const std::string& srcPath, const std::string& dstPath) {
    if (srcPath.empty() || dstPath.empty()) {
        throw EmptyPathException("Source or destination path is empty");
    }

    // =========================
    // Resolve source
    // =========================
    INode src = ResolvePath(srcPath);

    if (src.isDir()) {
        throw NotADirectoryException("Source is a directory");
    }

    // =========================
    // Read source file
    // =========================
    std::vector<char> data = ReadFile(srcPath);

    // =========================
    // Write destination file
    // =========================
    WriteFile(dstPath, std::move(data));
}

void Filesystem::MoveFile(const std::string& srcPath, const std::string& dstPath) {
    if (srcPath.empty() || dstPath.empty()) {
        throw EmptyPathException("Source or destination path is empty");
    }

    // Prevent noop / self-move
    if (srcPath == dstPath) {
        return;
    }

    // =========================
    // Validate source
    // =========================
    INode src = ResolvePath(srcPath);

    if (src.isDir()) {
        throw NotADirectoryException("Source is a directory");
    }

    // =========================
    // Copy first (safe)
    // =========================
    CopyFile(srcPath, dstPath);

    // =========================
    // Remove source
    // =========================
    RemoveFile(srcPath);
}

void Filesystem::RemoveFile(const std::string& path) {
    if (path.empty()) {
        throw EmptyPathException("Empty path");
    }

    // =========================
    // Resolve parent directory
    // =========================
    INode parent = ResolveParent(path);
    if (!parent.isDir()) {
        throw NotADirectoryException("Parent is not a directory");
    }

    auto parts = SplitPath(path);
    const std::string& filename = parts.back();

    // =========================
    // Find file inode
    // =========================
    auto fileIdOpt = FindChildId(parent, filename);
    if (!fileIdOpt) {
        throw PathNotFoundException("File not found: " + filename);
    }

    INode file = readINode(*fileIdOpt);

    if (file.isDir()) {
        throw NotADirectoryException("Cannot remove a directory with RemoveFile");
    }

    // =========================
    // Remove directory entry
    // =========================
    RemoveChild(parent, file.getId());
    writeINode(parent);

    // =========================
    // Free inode
    // =========================
    if (file.getLinks() == 1) {
        FreeNode(file);
    }
    else {
        file.removeLink();
    }
}

std::vector<std::pair<std::string, bool>>
Filesystem::GetSubdirectories(const std::string& path) const {
    if (path.empty()) {
        throw EmptyPathException("Empty path");
    }

    INode dir = ResolvePath(path);

    if (!dir.isDir()) {
        throw NotADirectoryException("Path is not a directory");
    }

    std::vector<std::pair<std::string, bool>> result;

    for (const auto& child : GetChildren(dir)) {
        // Optional: skip special entries
        if (child.name == "." || child.name == "..") {
            continue;
        }

        INode node = readINode(child.id);
        result.emplace_back(child.name, node.isDir());
    }

    return result;
}

void Filesystem::ChangeActiveDirectory(const std::string &path) {
    if (path.empty()) {
        throw EmptyPathException("Empty path");
    }

    INode dir = ResolvePath(path);

    if (!dir.isDir()) {
        throw NotADirectoryException("Path is not a directory");
    }
    this->currentNode = dir;
}

std::vector<std::string> Filesystem::GetCurrentPath() const {
    std::vector<std::string> path;

    INode node = currentNode;

    // Root → empty path
    if (node.getId() == superblock.rootNodeId) {
        return path;
    }

    while (true) {
        // Find parent
        auto parentId = FindChildId(node, "..");
        if (!parentId) {
            throw NoParentDirectoryException("Directory has no parent");
        }

        INode parent = readINode(*parentId);

        // Root reached
        if (parent.getId() == node.getId()) {
            break;
        }

        // Find name of current node in parent
        bool found = false;
        for (const auto& child : GetChildren(parent)) {
            if (child.id == node.getId() && child.name != "." && child.name != "..") {
                path.push_back(child.name);
                found = true;
                break;
            }
        }

        if (!found) {
            throw FileReadException("Failed to resolve current path");
        }

        node = parent;
    }

    std::reverse(path.begin(), path.end());
    return path;
}

std::string Filesystem::GetNodeInfo(const std::string& path) const {
    if (path.empty()) {
        throw EmptyPathException("Empty path");
    }

    INode node = ResolvePath(path);

    // =========================
    // Extract name
    // =========================
    std::string name;
    if (path == "/") {
        name = "/";
    } else {
        auto parts = SplitPath(path);
        name = parts.back();
    }

    std::ostringstream out;

    // name
    out << name;

    // size
    out << " – " << node.getSize() << " B";

    // inode id
    out << " – i-uzel " << node.getId();

    // =========================
    // Direct blocks
    // =========================
    out << " – přímé odkazy ";

    bool first = true;
    for (uint32_t block : node.getDirectLinks()) {
        if (block == INode::UNUSED_LINK) {
            continue;
        }

        if (!first) {
            out << ", ";
        }
        out << block;
        first = false;
    }

    if (first) {
        out << "žádné";
    }

    // =========================
    // Indirect info (optional)
    // =========================
    if (node.getFirstLevelIndirectLink() != INode::UNUSED_LINK) {
        out << " – nepřímý 1. úrovně " << node.getFirstLevelIndirectLink();
    }

    if (node.getSecondLevelIndirectLink() != INode::UNUSED_LINK) {
        out << " – nepřímý 2. úrovně " << node.getSecondLevelIndirectLink();
    }

    if (!node.isDir()) {
        out << " – hardlinky " << node.getLinks();
    }
    return out.str();
}

void Filesystem::LinkFile(const std::string& originalPath,
                          const std::string& linkPath) {
    if (originalPath.empty() || linkPath.empty()) {
        throw EmptyPathException("Source or link path is empty");
    }

    // =========================
    // Resolve original
    // =========================
    INode original = ResolvePath(originalPath);

    if (original.isDir()) {
        throw NotADirectoryException("Cannot hard-link a directory");
    }

    // =========================
    // Resolve destination parent
    // =========================
    INode parent = ResolveParent(linkPath);
    if (!parent.isDir()) {
        throw NotADirectoryException("Link parent is not a directory");
    }

    auto parts = SplitPath(linkPath);
    const std::string& linkName = parts.back();

    if (linkName.empty()) {
        throw PathNotFoundException("Invalid link name");
    }

    // Destination must not exist
    if (FindChildId(parent, linkName)) {
        throw FileWriteException("Destination already exists");
    }

    // =========================
    // Create directory entry
    // =========================
    AddChild(parent, linkName, original.getId());
    original.addLink();
    writeINode(original);
    writeINode(parent);
}

std::string Filesystem::GetFilesystemStats() const {
    if (!formated) {
        throw FilesystemNotFormattedException("Filesystem is not formatted");
    }

    std::ostringstream out;

    // =========================
    // Basic geometry
    // =========================
    out << "Velikost FS: " << superblock.size << " B\n";
    out << "Velikost bloku: " << superblock.blockSize << " B\n";

    // =========================
    // Block stats
    // =========================
    uint32_t usedBlocks = 0;
    for (uint32_t i = 0; i < superblock.totalBlocks; ++i) {
        if (BlockBitmap.Get(i)) {
            ++usedBlocks;
        }
    }

    uint32_t freeBlocks = superblock.totalBlocks - usedBlocks;

    out << "Bloky: celkem " << superblock.totalBlocks
        << ", použito " << usedBlocks
        << ", volné " << freeBlocks << "\n";

    // =========================
    // Inode stats
    // =========================
    uint32_t usedInodes = 0;
    for (uint32_t i = 0; i < superblock.totalInodes; ++i) {
        if (INodeBitmap.Get(i)) {
            ++usedInodes;
        }
    }

    uint32_t freeInodes = superblock.totalInodes - usedInodes;

    out << "I-uzly: celkem " << superblock.totalInodes
        << ", použito " << usedInodes
        << ", volné " << freeInodes << "\n";

    // =========================
    // Root inode
    // =========================
    out << "Kořenový i-uzel: " << superblock.rootNodeId << "\n";

    // =========================
    // Current directory
    // =========================
    out << "Aktuální adresář: ";

    auto cwd = GetCurrentPath();
    if (cwd.empty()) {
        out << "/";
    } else {
        out << "/";
        for (size_t i = 0; i < cwd.size(); ++i) {
            out << cwd[i];
            if (i + 1 < cwd.size()) {
                out << "/";
            }
        }
    }

    out << "\n";

    return out.str();
}
