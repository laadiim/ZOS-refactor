//
// Created by laadim on 18.01.26.
//

#include "FileIOHandler.h"

#include <filesystem>

#include "Exceptions.h"

FileIOHandler::~FileIOHandler() {
    this->Flush();
    this->CloseFile();
}

void FileIOHandler::OpenFile(const std::string& fileName, const FileModes mode) {
    std::ios::openmode flags = std::ios::binary;

    switch (mode) {
    case FileModes::READ:
        if (!std::filesystem::exists(fileName)) {
            throw FileDoesNotExistException("File does not exist: " + fileName);
        }
        flags |= std::ios::in;
        break;

    case FileModes::READ_WRITE:
        flags |= std::ios::in | std::ios::out | std::ios::trunc;
        break;
    }

    auto stream = std::make_unique<std::fstream>(fileName, flags);

    if (!stream->is_open()) {
        throw CouldNotOpenFileException("Could not open file: " + fileName);
    }

    this->stream = std::move(stream);
    this->fileName = fileName;
    this->mode = mode;
}

void FileIOHandler::CloseFile() const {
    this->Flush();
    this->stream->close();
}



std::vector<char> FileIOHandler::ReadBytes(const uint64_t offset, const uint64_t size) const {
    if (!stream || !stream->is_open()) {
        throw FileNotOpenException("File is not open");
    }

    std::vector<char> buffer(size);

    stream->clear();
    stream->seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    stream->read(buffer.data(), static_cast<std::streamsize>(size));

    buffer.resize(stream->gcount());
    return buffer;
}

uint64_t FileIOHandler::WriteBytes(const uint64_t offset, const std::vector<char> &data) const {
    EnsureWritable();

    if (!stream || !stream->is_open()) {
        throw FileNotOpenException("File is not open");
    }

    stream->clear();
    stream->seekp(static_cast<std::streamoff>(offset), std::ios::beg);
    stream->write(data.data(), static_cast<std::streamsize>(data.size()));

    if (!(*stream)) {
        throw FileWriteException("Failed to write bytes");
    }

    return data.size();
}

void FileIOHandler::Flush() const {
    this->stream->flush();
}

uint64_t FileIOHandler::Resize(const uint64_t newSize) const {
    EnsureWritable();

    if (!stream || !stream->is_open()) {
        throw FileNotOpenException("File is not open");
    }

    stream->flush();

    std::error_code ec;
    std::filesystem::resize_file(fileName, newSize, ec);

    if (ec) {
        throw FileWriteException("Failed to resize file: " + ec.message());
    }

    // After external resize, reset stream state & position
    stream->clear();
    stream->seekg(0, std::ios::beg);
    stream->seekp(0, std::ios::beg);

    return newSize;
}

bool FileIOHandler::IsOpen() const {
    return stream->is_open();
}

void FileIOHandler::EnsureWritable() const {
    if (mode == FileModes::READ) {
        throw FileReadOnlyException("File opened read-only");
    }
}
