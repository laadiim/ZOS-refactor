//
// Created by laadim on 26.01.26.
//

#pragma once
#include <array>
#include <cstdint>
#include <vector>

/**
 * @class INode
 * @brief Inode structure representing a filesystem object.
 *
 * An inode describes either a file or a directory and stores:
 *  - object identity
 *  - file size
 *  - hard link count
 *  - direct and indirect data block references
 *
 * The inode is serialized to a fixed-size on-disk layout
 * and reconstructed when loading the filesystem.
 */
class INode {
public:
    /** Number of direct block references stored in the inode. */
    static constexpr int DIRECT_LINKS = 5;

    /** Size of the serialized inode in bytes. */
    static constexpr int BYTES = 41;

    /** Marker value for an unused block reference. */
    static constexpr uint32_t UNUSED_LINK = UINT32_MAX;

    // =====================================================
    // Serialization
    // =====================================================

    /**
     * @brief Deserialize an inode from raw byte data.
     *
     * @param bytes Raw inode bytes read from disk.
     * @return Reconstructed inode instance.
     */
    static INode FromBytes(std::vector<char> bytes);

    /**
     * @brief Serialize inode to raw byte data.
     *
     * @return Byte representation suitable for disk storage.
     */
    [[nodiscard]] std::vector<char> ToBytes() const;

    // =====================================================
    // Construction / destruction
    // =====================================================

    /**
     * @brief Construct a new inode.
     *
     * @param id Unique inode identifier.
     * @param isDir True if inode represents a directory.
     */
    INode(uint32_t id, bool isDir);

    /**
     * @brief Default constructor.
     *
     * Creates an uninitialized inode instance.
     */
    INode();

    /**
     * @brief Destructor.
     */
    ~INode();

    // =====================================================
    // Basic properties
    // =====================================================

    /**
     * @brief Get inode identifier.
     */
    [[nodiscard]] uint32_t getId() const;

    /**
     * @brief Check whether inode represents a directory.
     */
    [[nodiscard]] bool isDir() const;

    /**
     * @brief Get number of hard links referencing this inode.
     */
    [[nodiscard]] uint32_t getLinks() const;

    /**
     * @brief Increment hard link count.
     */
    void addLink();

    /**
     * @brief Decrement hard link count.
     *
     * @return True if link count reached zero.
     */
    bool removeLink();

    /**
     * @brief Get file size in bytes.
     */
    [[nodiscard]] uint32_t getSize() const;

    /**
     * @brief Increase file size.
     *
     * @param bytes Number of bytes to add.
     */
    void addSize(uint32_t bytes);

    /**
     * @brief Decrease file size.
     *
     * @param bytes Number of bytes to remove.
     */
    void removeSize(uint32_t bytes);

    // =====================================================
    // Direct block references
    // =====================================================

    /**
     * @brief Get direct data block references.
     */
    [[nodiscard]]
    std::array<uint32_t, DIRECT_LINKS> getDirectLinks() const;

    /**
     * @brief Add a direct data block reference.
     *
     * @param link Block identifier.
     */
    void addDirectLink(uint32_t link);

    /**
     * @brief Remove a direct data block reference.
     *
     * @param link Block identifier.
     */
    void removeDirectLink(uint32_t link);

    /**
     * @brief Clear all direct block references.
     */
    void clearDirectLinks();

    // =====================================================
    // Indirect block references
    // =====================================================

    /**
     * @brief Get first-level indirect block reference.
     */
    [[nodiscard]] uint32_t getFirstLevelIndirectLink() const;

    /**
     * @brief Set first-level indirect block reference.
     *
     * @param link Block identifier.
     */
    void addFirstLevelIndirectLink(uint32_t link);

    /**
     * @brief Remove first-level indirect block reference.
     */
    void removeFirstLevelIndirectLink();

    /**
     * @brief Get second-level indirect block reference.
     */
    [[nodiscard]] uint32_t getSecondLevelIndirectLink() const;

    /**
     * @brief Set second-level indirect block reference.
     *
     * @param link Block identifier.
     */
    void addSecondLevelIndirectLink(uint32_t link);

    /**
     * @brief Remove second-level indirect block reference.
     */
    void removeSecondLevelIndirectLink();

    // =====================================================
    // On-disk layout
    // =====================================================

    /**
     * @brief Serialized inode layout (byte offsets).
     *
     * offset | field
     * -------|----------------
     *      0 | inode id
     *      4 | hard link count
     *      8 | file size
     *     12 | direct[0]
     *     16 | direct[1]
     *     20 | direct[2]
     *     24 | direct[3]
     *     28 | direct[4]
     *     32 | indirect level 1
     *     36 | indirect level 2
     *     40 | is directory flag
     * -------|----------------
     * TOTAL: 41 bytes
     */
private:
    /** Inode identifier. */
    uint32_t _id;

    /** Hard link reference count. */
    uint32_t _links;

    /** True if inode represents a directory. */
    bool _isDir;

    /** File size in bytes. */
    uint32_t _size;

    /** Direct data block references. */
    std::array<uint32_t, DIRECT_LINKS> _direct;

    /** First-level indirect block reference. */
    uint32_t _indirect1;

    /** Second-level indirect block reference. */
    uint32_t _indirect2;
};
