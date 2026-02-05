//
// Created by laadim on 18.01.26.
//

#pragma once
#include <cstdint>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

/**
 * @brief Stream-based file I/O handler.
 *
 * Provides low-level binary read/write access to a file using a single
 * managed stream. Supports file creation, resizing, and random-access
 * reads and writes.
 */
class FileIOHandler {
public:
    /**
     * @brief File opening modes.
     */
    enum class FileModes {
        /// Open file in read-only mode
        READ,

        /// Open file in read-write mode (creates file if missing)
        READ_WRITE
    };

    /**
     * @brief Construct a FileIOHandler.
     *
     * Does not open a file by itself.
     */
    FileIOHandler() = default;

    /**
     * @brief Destructor.
     *
     * Flushes buffers and closes the file stream if open.
     */
    ~FileIOHandler();

    /**
     * @brief Open a file stream.
     *
     * Opens a binary stream over the specified file.
     * In READ_WRITE mode, the file is created if it does not exist.
     *
     * @param fileName Path to the file.
     * @param mode Mode to open the file in.
     *
     * @throws FileDoesNotExistException If the file does not exist in READ mode.
     * @throws CouldNotOpenFileException If the file could not be opened.
     */
    void OpenFile(const std::string& fileName, FileModes mode);

    /**
     * @brief Flush and close the file stream.
     *
     * Safe to call multiple times.
     */
    void CloseFile() const;

    /**
     * @brief Ensure the file was opened in a writable mode.
     *
     * @throws FileReadOnlyException If the file is read-only.
     */
    void EnsureWritable() const;

    /**
     * @brief Read a sequence of bytes from the file.
     *
     * @param offset Byte offset from the beginning of the file.
     * @param size Number of bytes to read.
     *
     * @return Vector containing the bytes read.
     *
     * @throws FileNotOpenException If no file is open.
     */
    [[nodiscard]] std::vector<char> ReadBytes(uint64_t offset, uint64_t size) const;

    /**
     * @brief Write bytes to the file.
     *
     * @param offset Byte offset from the beginning of the file.
     * @param data Data to write.
     *
     * @throws FileNotOpenException If no file is open.
     * @throws FileReadOnlyException If file is read-only.
     * @throws FileWriteException If the write fails.
     */
    void WriteBytes(uint64_t offset, const std::vector<char>& data) const;

    /**
     * @brief Flush buffered output to disk.
     */
    void Flush() const;

    /**
     * @brief Resize the currently open file.
     *
     * The file is resized and fully zero-filled to the new size.
     *
     * @param newSize Target file size in bytes.
     *
     * @return The new file size.
     *
     * @throws FileNotOpenException If no file is open.
     * @throws FileReadOnlyException If file is read-only.
     * @throws FileWriteException If resizing or zero-filling fails.
     */
    [[nodiscard]] uint64_t Resize(uint64_t newSize) const;

    /**
     * @brief Check whether a file stream is currently open.
     *
     * @return True if a file is open, false otherwise.
     */
    [[nodiscard]] bool IsOpen() const;

private:
    /// Path of the currently opened file
    std::string fileName;

    /// Owned file stream
    std::unique_ptr<std::fstream> stream;

    /// Mode the file was opened with
    FileModes mode;
};
