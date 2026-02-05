//
// Created by laadim on 18.01.26.
//

#include "FileIOHandler.h"

#include <array>
#include <filesystem>

#include "FileIOExceptions.h"

/**
 * @brief Destructor.
 *
 * Ensures all buffered data is flushed and the file is closed.
 */
FileIOHandler::~FileIOHandler() {
    this->Flush();
    this->CloseFile();
}

/**
 * @brief Open a file stream with the given mode.
 */
void FileIOHandler::OpenFile(const std::string& fileName, const FileModes mode) {
    // Always operate in binary mode
    std::ios::openmode flags = std::ios::binary;

    switch (mode) {
        case FileModes::READ:
            // Read-only mode requires the file to already exist
            if (!std::filesystem::exists(fileName)) {
                throw FileDoesNotExistException("File does not exist: " + fileName);
            }
            flags |= std::ios::in;
            break;

        case FileModes::READ_WRITE:
            // Read-write mode creates the file if missing
            flags |= std::ios::in | std::ios::out;
            break;
    }

    // Create and attempt to open the stream
    auto stream = std::make_unique<std::fstream>(fileName, flags);

    if (!stream->is_open()) {
        throw CouldNotOpenFileException("Could not open file: " + fileName);
    }

    // Commit state only after successful open
    this->stream = std::move(stream);
    this->fileName = fileName;
    this->mode = mode;
}

/**
 * @brief Flush buffers and close the open file stream.
 */
void FileIOHandler::CloseFile() const {
    this->Flush();
    this->stream->close();
}

/**
 * @brief Read bytes from the file at a specific offset.
 */
std::vector<char> FileIOHandler::ReadBytes(const uint64_t offset,
                                           const uint64_t size) const {
    // Validate stream state
    if (!this->stream || !this->stream->is_open()) {
        throw FileNotOpenException("File is not open");
    }

    // Allocate buffer of requested size
    std::vector<char> buffer(size);

    // Seek to requested position and read
    this->stream->seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    this->stream->read(buffer.data(),
                       static_cast<std::streamsize>(size));

    // Shrink buffer to actual number of bytes read
    buffer.resize(this->stream->gcount());
    return buffer;
}

/**
 * @brief Write bytes to the file at a specific offset.
 */
void FileIOHandler::WriteBytes(const uint64_t offset,
                               const std::vector<char>& data) const {
    // Ensure file is writable
    this->EnsureWritable();

    if (!this->stream || !this->stream->is_open()) {
        throw FileNotOpenException("File is not open");
    }

    // Clear any previous error flags
    this->stream->clear();

    // Seek and write data
    this->stream->seekp(static_cast<std::streamoff>(offset), std::ios::beg);
    this->stream->write(data.data(),
                        static_cast<std::streamsize>(data.size()));

    // Verify write success
    if (!(*this->stream)) {
        throw FileWriteException("Failed to write bytes");
    }
}

/**
 * @brief Flush buffered output to disk.
 */
void FileIOHandler::Flush() const {
    this->stream->flush();
}

/**
 * @brief Resize the currently open file and zero-fill it.
 */
uint64_t FileIOHandler::Resize(const uint64_t newSize) const {
    this->EnsureWritable();

    if (!this->stream || !this->stream->is_open()) {
        throw FileNotOpenException("File is not open");
    }

    // Flush and close before resizing
    this->stream->flush();
    this->stream->close();

    // Perform filesystem resize
    std::error_code ec;
    std::filesystem::resize_file(this->fileName, newSize, ec);
    if (ec) {
        throw FileWriteException("Failed to resize file: " + ec.message());
    }

    // Reopen the resized file
    this->stream->open(this->fileName,
                       std::ios::in | std::ios::out | std::ios::binary);
    if (!this->stream->is_open()) {
        throw FileNotOpenException("Failed to reopen file after resize");
    }

    // Zero-fill the entire file
    constexpr std::size_t ZERO_BUF_SIZE = 4096;
    std::array<char, ZERO_BUF_SIZE> zeroBuf{};
    zeroBuf.fill(0);

    uint64_t remaining = newSize;
    this->stream->seekp(0, std::ios::beg);

    while (remaining > 0) {
        const auto toWrite =
            static_cast<std::size_t>(
                std::min<uint64_t>(remaining, ZERO_BUF_SIZE));

        this->stream->write(zeroBuf.data(), toWrite);
        if (!(*this->stream)) {
            throw FileWriteException("Failed to zero-fill file");
        }

        remaining -= toWrite;
    }

    // Final flush and reset state
    this->stream->flush();
    this->stream->clear();
    this->stream->seekg(0, std::ios::beg);
    this->stream->seekp(0, std::ios::beg);

    return newSize;
}

/**
 * @brief Check whether a file stream is open.
 */
bool FileIOHandler::IsOpen() const {
    return this->stream && this->stream->is_open();
}

/**
 * @brief Ensure the file was opened in a writable mode.
 */
void FileIOHandler::EnsureWritable() const {
    if (this->mode == FileModes::READ) {
        throw FileReadOnlyException("File opened read-only");
    }
}
