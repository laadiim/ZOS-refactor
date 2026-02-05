//
// Created by laadim on 26.01.26.
//

#pragma once
#include <cstdint>
#include <string>

/**
 * @brief Represents a mapping between a child node's name and its inode ID.
 *
 * This structure is typically used for directory entries, where each
 * child is identified by a human-readable name and a unique inode ID.
 */
struct ChildNodeNameIdPair {
    /** @brief Name of the child node (file or directory). */
    std::string name;

    /** @brief Inode ID associated with the child node. */
    uint32_t id;
};
