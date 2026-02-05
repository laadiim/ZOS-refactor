//
// Created by laadim on 18.01.26.
//

#pragma once
#include <exception>
#include <stdexcept>
#include <string>

/**
 * @brief Exception thrown when a requested file does not exist.
 *
 * Typically used when attempting to open or access a file
 * that cannot be found at the given path.
 */
class FileDoesNotExistException : public std::runtime_error {
public:
    /**
     * @brief Construct a new FileDoesNotExistException.
     *
     * @param msg Human-readable error message.
     */
    explicit FileDoesNotExistException(const std::string &msg)
        : std::runtime_error{msg} {}
};

/**
 * @brief Exception thrown when a file cannot be opened.
 *
 * This may occur due to insufficient permissions, invalid path,
 * or underlying OS-level errors.
 */
class CouldNotOpenFileException : public std::runtime_error {
public:
    /**
     * @brief Construct a new CouldNotOpenFileException.
     *
     * @param msg Human-readable error message.
     */
    explicit CouldNotOpenFileException(const std::string &msg)
        : std::runtime_error{msg} {}
};

/**
 * @brief Exception thrown when an operation is attempted on a file
 *        that is not currently open.
 */
class FileNotOpenException : public std::runtime_error {
public:
    /**
     * @brief Construct a new FileNotOpenException.
     *
     * @param msg Human-readable error message.
     */
    explicit FileNotOpenException(const std::string &msg)
        : std::runtime_error{msg} {}
};

/**
 * @brief Exception thrown when attempting to write to a read-only file.
 */
class FileReadOnlyException : public std::runtime_error {
public:
    /**
     * @brief Construct a new FileReadOnlyException.
     *
     * @param msg Human-readable error message.
     */
    explicit FileReadOnlyException(const std::string &msg)
        : std::runtime_error{msg} {}
};

/**
 * @brief Exception thrown when a file read operation fails.
 *
 * This may indicate I/O errors, unexpected EOF, or corrupted data.
 */
class FileReadException : public std::runtime_error {
public:
    /**
     * @brief Construct a new FileReadException.
     *
     * @param msg Human-readable error message.
     */
    explicit FileReadException(const std::string &msg)
        : std::runtime_error{msg} {}
};

/**
 * @brief Exception thrown when a file write operation fails.
 *
 * This may indicate I/O errors, insufficient space, or permission issues.
 */
class FileWriteException : public std::runtime_error {
public:
    /**
     * @brief Construct a new FileWriteException.
     *
     * @param msg Human-readable error message.
     */
    explicit FileWriteException(const std::string &msg)
        : std::runtime_error{msg} {}
};
