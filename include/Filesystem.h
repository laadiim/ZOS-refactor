//
// Created by laadim on 26.01.26.
//

#pragma once
#include <cstdint>
#include <memory>
#include <vector>

#include "Bitmap.h"
#include "INode.h"
#include "Superblock.h"
#include "../helpers/ChildNodeNameIdPair.h"
#include "../helpers/FileIOHandler.h"

/**
 * @class Filesystem
 * @brief Block-based filesystem stored in a single image file.
 *
 * Provides a hierarchical filesystem supporting:
 *  - directories
 *  - regular files
 *  - hard links
 *
 * The filesystem uses:
 *  - a superblock for metadata
 *  - an inode table
 *  - bitmaps for inode and block allocation
 *
 * All metadata is flushed back to disk on destruction.
 */
class Filesystem {
public:
    /**
     * @brief Open or mount a filesystem image.
     *
     * If the image contains a valid superblock, the filesystem
     * is mounted. Otherwise it remains unformatted.
     *
     * @param imagePath Path to the filesystem image file.
     */
    explicit Filesystem(const std::string& imagePath);

    /**
     * @brief Flush metadata and close the filesystem image.
     */
    ~Filesystem();

    /**
     * @brief Format the filesystem image.
     *
     * Initializes a new filesystem layout and overwrites
     * any existing data in the image.
     *
     * @param bytes Desired filesystem image size in bytes.
     */
    void Format(uint32_t bytes);

    /**
     * @brief Check whether the filesystem is formatted.
     *
     * @return True if the filesystem is formatted.
     */
    [[nodiscard]] bool Formated() const;

    // =========================
    // Directory operations
    // =========================

    /**
     * @brief Create a new directory.
     *
     * @param path Path of the directory to create.
     */
    void CreateDirectory(const std::string& path);

    /**
     * @brief Remove an empty directory.
     *
     * @param path Path of the directory to remove.
     */
    void RemoveDirectory(const std::string& path);

    // =========================
    // File operations
    // =========================

    /**
     * @brief Write data to a file.
     *
     * Creates the file if it does not exist or overwrites
     * existing contents if it does.
     *
     * @param srcPath Path of the file.
     * @param data File contents.
     */
    void WriteFile(const std::string& srcPath, std::vector<char> data);

    /**
     * @brief Read contents of a file.
     *
     * @param srcPath Path of the file.
     * @return File contents.
     */
    [[nodiscard]] std::vector<char> ReadFile(const std::string& srcPath);

    /**
     * @brief Copy a file.
     *
     * @param srcPath Source file path.
     * @param dstPath Destination file path.
     */
    void CopyFile(const std::string& srcPath, const std::string& dstPath);

    /**
     * @brief Move a file.
     *
     * Equivalent to copy followed by remove.
     *
     * @param srcPath Source file path.
     * @param dstPath Destination file path.
     */
    void MoveFile(const std::string& srcPath, const std::string& dstPath);

    /**
     * @brief Remove a file.
     *
     * @param path Path of the file to remove.
     */
    void RemoveFile(const std::string& path);

    /**
     * @brief Create a hard link to a file.
     *
     * @param originalPath Path of the original file.
     * @param linkPath Path of the new link.
     */
    void LinkFile(const std::string& originalPath,
                  const std::string& linkPath);

    // =========================
    // Navigation & queries
    // =========================

    /**
     * @brief List contents of a directory.
     *
     * @param path Path of the directory.
     * @return Vector of (name, isDirectory) pairs.
     */
    [[nodiscard]]
    std::vector<std::pair<std::string, bool>>
    GetSubdirectories(const std::string& path) const;

    /**
     * @brief Change the current working directory.
     *
     * @param path Path to the new current directory.
     */
    void ChangeActiveDirectory(const std::string& path);

    /**
     * @brief Get the current working directory path.
     *
     * @return Vector of path components from root.
     */
    [[nodiscard]] std::vector<std::string> GetCurrentPath() const;

    /**
     * @brief Get human-readable information about a node.
     *
     * @param path Path of the node.
     * @return Information string.
     */
    [[nodiscard]] std::string GetNodeInfo(const std::string& path) const;

    /**
     * @brief Get filesystem statistics.
     *
     * @return Human-readable filesystem statistics.
     */
    [[nodiscard]] std::string GetFilesystemStats() const;

private:
    /// File I/O handler for the filesystem image
    std::unique_ptr<FileIOHandler> FileIO;

    /// Filesystem superblock
    Superblock superblock{};

    /// Bitmap tracking inode allocation
    Bitmap INodeBitmap;

    /// Bitmap tracking data block allocation
    Bitmap BlockBitmap;

    /// Current working directory inode
    INode currentNode;

    /// Path to filesystem image
    std::string imagePath;

    /// Expected filesystem magic value
    uint32_t magic = 0xDEADBEEF;

    /// Indicates whether the filesystem is formatted
    bool formated = false;

    // =========================
    // Internal helpers
    // =========================

    /**
     * @brief Read an inode from disk.
     */
    [[nodiscard]] INode readINode(uint32_t id) const;

    /**
     * @brief Write an inode to disk.
     */
    void writeINode(const INode& node) const;

    /**
     * @brief Allocate a new inode.
     */
    std::optional<INode> AllocateNode(bool isDir = false);

    /**
     * @brief Free an inode and its blocks.
     */
    void FreeNode(const INode& node);

    /**
     * @brief Allocate a data block.
     */
    std::optional<uint32_t> AllocateBlock();

    /**
     * @brief Free a data block.
     */
    void FreeBlock(uint32_t block);

    /**
     * @brief Add a directory entry.
     */
    void AddChild(INode& node,
                  std::string name,
                  uint32_t childNode);

    /**
     * @brief Get all directory entries of a node.
     */
    [[nodiscard]]
    std::vector<ChildNodeNameIdPair>
    GetChildren(const INode& node) const;

    /**
     * @brief Remove a directory entry.
     */
    void RemoveChild(const INode& node,
                     uint32_t childNode, std::string filename) const;

    /**
     * @brief Attach a data block to an inode.
     */
    void AttachBlock(INode& node, uint32_t block);

    /**
     * @brief Detach a data block from an inode.
     */
    void DeattachBlock(INode& node, uint32_t block);

    /**
     * @brief Interpret a block as directory entries.
     */
    [[nodiscard]]
    std::vector<ChildNodeNameIdPair>
    ReadBlockAsSubdirectories(uint32_t block) const;

    /**
     * @brief Interpret a block as a block-ID table.
     */
    [[nodiscard]]
    std::vector<uint32_t>
    ReadBlockAsBlockIds(uint32_t block) const;

    /**
     * @brief Remove an entry from a block-ID table.
     */
    bool RemoveFromBlockIdTable(uint32_t tableBlock,
                               uint32_t value);

    /**
     * @brief Find a child inode by name.
     */
    [[nodiscard]]
    std::optional<uint32_t>
    FindChildId(const INode& dir,
                std::string name) const;

    /**
     * @brief Check whether a directory contains a child.
     */
    [[nodiscard]]
    bool ExistsChild(const INode& dir,
                     const std::string& name) const;

    /**
     * @brief Get all data blocks used by an inode.
     */
    [[nodiscard]]
    std::vector<uint32_t>
    GetAllBlockIds(const INode& node) const;

    /**
     * @brief Resolve a filesystem path to an inode.
     */
    [[nodiscard]]
    INode ResolvePath(const std::string& path) const;

    /**
     * @brief Resolve parent directory of a path.
     */
    [[nodiscard]]
    INode ResolveParent(const std::string& path) const;
};
