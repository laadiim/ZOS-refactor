//
// Created by laadim on 04.02.26.
//

#pragma once
#include <string>
#include <vector>

/**
 * @brief Split a filesystem path into its individual components.
 *
 * Splits the given path on the '/' separator and returns only
 * non-empty path segments.
 *
 * Examples:
 *  - "/a/b/c"   -> {"a", "b", "c"}
 *  - "a/b/c"    -> {"a", "b", "c"}
 *  - "/a//b/"   -> {"a", "b"}
 *  - "/"        -> {}
 *
 * @param path Input filesystem path.
 * @return Vector of path components.
 */
inline std::vector<std::string> SplitPath(const std::string& path) {
    std::vector<std::string> parts;
    std::string current;

    // Iterate character-by-character to avoid temporary allocations
    for (char c : path) {
        if (c == '/') {
            // Commit current segment when a separator is encountered
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
        } else {
            // Accumulate path characters
            current.push_back(c);
        }
    }

    // Push the final segment, if any
    if (!current.empty()) {
        parts.push_back(current);
    }

    return parts;
}
