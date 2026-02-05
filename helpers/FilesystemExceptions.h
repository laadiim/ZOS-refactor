//
// Created by laadim on 05.02.26.
//

#pragma once
#include <stdexcept>
#include <string>

// =========================
// Generic filesystem errors
// =========================

/**
 * @brief Thrown when an operation is performed on an unformatted filesystem.
 */
class FilesystemNotFormattedException : public std::runtime_error {
public:
    /**
     * @param msg Human-readable error message.
     */
    explicit FilesystemNotFormattedException(const std::string& msg)
        : std::runtime_error{msg} {}
};

/**
 * @brief Thrown when the filesystem image size is invalid.
 */
class InvalidFilesystemSizeException : public std::runtime_error {
public:
    /**
     * @param msg Human-readable error message.
     */
    explicit InvalidFilesystemSizeException(const std::string& msg)
        : std::runtime_error{msg} {}
};

/**
 * @brief Thrown when a filesystem image cannot be resized.
 */
class CouldNotResizeImageException : public std::runtime_error {
public:
    /**
     * @param msg Human-readable error message.
     */
    explicit CouldNotResizeImageException(const std::string& msg)
        : std::runtime_error{msg} {}
};

/**
 * @brief Thrown when the filesystem superblock is invalid or corrupted.
 */
class InvalidSuperblockException : public std::runtime_error {
public:
    /**
     * @param msg Human-readable error message.
     */
    explicit InvalidSuperblockException(const std::string& msg)
        : std::runtime_error{msg} {}
};

// =========================
// Storage / IO layout errors
// =========================

/**
 * @brief Thrown when an inode has an invalid or unsupported size.
 */
class InvalidINodeSizeException : public std::runtime_error {
public:
    /**
     * @param msg Human-readable error message.
     */
    explicit InvalidINodeSizeException(const std::string& msg)
        : std::runtime_error{msg} {}
};

/**
 * @brief Thrown when a data block has an invalid or unsupported size.
 */
class InvalidBlockSizeException : public std::runtime_error {
public:
    /**
     * @param msg Human-readable error message.
     */
    explicit InvalidBlockSizeException(const std::string& msg)
        : std::runtime_error{msg} {}
};

// =========================
// Allocation / capacity errors
// =========================

/**
 * @brief Thrown when an inode cannot be allocated.
 */
class CouldNotAllocateNodeException : public std::runtime_error {
public:
    /**
     * @param msg Human-readable error message.
     */
    explicit CouldNotAllocateNodeException(const std::string& msg)
        : std::runtime_error{msg} {}
};

/**
 * @brief Thrown when a data block cannot be allocated.
 */
class CouldNotAllocateBlockException : public std::runtime_error {
public:
    /**
     * @param msg Human-readable error message.
     */
    explicit CouldNotAllocateBlockException(const std::string& msg)
        : std::runtime_error{msg} {}
};

/**
 * @brief Thrown when a file exceeds the maximum representable size.
 */
class FileTooLargeException : public std::runtime_error {
public:
    /**
     * @param msg Human-readable error message.
     */
    explicit FileTooLargeException(const std::string& msg)
        : std::runtime_error{msg} {}
};

// =========================
// Directory / path errors
// =========================

/**
 * @brief Thrown when an empty path is provided.
 */
class EmptyPathException : public std::runtime_error {
public:
    /**
     * @param msg Human-readable error message.
     */
    explicit EmptyPathException(const std::string& msg)
        : std::runtime_error{msg} {}
};

/**
 * @brief Thrown when a filesystem path cannot be resolved.
 */
class PathNotFoundException : public std::runtime_error {
public:
    /**
     * @param msg Human-readable error message.
     */
    explicit PathNotFoundException(const std::string& msg)
        : std::runtime_error{msg} {}
};

/**
 * @brief Thrown when an operation expects a directory but finds a file.
 */
class NotADirectoryException : public std::runtime_error {
public:
    /**
     * @param msg Human-readable error message.
     */
    explicit NotADirectoryException(const std::string& msg)
        : std::runtime_error{msg} {}
};

/**
 * @brief Thrown when a parent directory does not exist.
 */
class NoParentDirectoryException : public std::runtime_error {
public:
    /**
     * @param msg Human-readable error message.
     */
    explicit NoParentDirectoryException(const std::string& msg)
        : std::runtime_error{msg} {}
};

/**
 * @brief Thrown when a child entry cannot be found in a directory.
 */
class ChildNotFoundException : public std::runtime_error {
public:
    /**
     * @param msg Human-readable error message.
     */
    explicit ChildNotFoundException(const std::string& msg)
        : std::runtime_error{msg} {}
};

// =========================
// Block / linking errors
// =========================

/**
 * @brief Thrown when a data block is not attached to an inode.
 */
class BlockNotAttachedException : public std::runtime_error {
public:
    /**
     * @param msg Human-readable error message.
     */
    explicit BlockNotAttachedException(const std::string& msg)
        : std::runtime_error{msg} {}
};