//
// Created by laadim on 18.01.26.
//
#include <cassert>
#include <iostream>
#include <filesystem>

#include "helpers/FileIOHandler.h"
#include "helpers/Exceptions.h"


void TestFileIOHandler() {
    const std::string testFile = "file_io_test.bin";

    // Clean slate
    std::filesystem::remove(testFile);

    FileIOHandler io;

    // ---- 1️⃣ Open for READ_WRITE (should create file) ----
    io.OpenFile(testFile, FileIOHandler::FileModes::READ_WRITE);
    assert(std::filesystem::exists(testFile));

    // ---- 2️⃣ Write bytes ----
    std::vector<char> data = { 'A', 'B', 'C', 'D' };
    uint64_t written = io.WriteBytes(0, data);
    assert(written == data.size());

    // ---- 3️⃣ Read bytes back ----
    auto readBack = io.ReadBytes(0, 4);
    assert(readBack == data);

    // ---- 5️⃣ Resize larger ----
    io.Resize(10);
    auto grown = io.ReadBytes(0, 10);
    assert(grown.size() == 10);

    // Newly extended bytes should be zero
    for (size_t i = 4; i < 10; ++i) {
        assert(grown[i] == 0);
    }

    // ---- 6️⃣ Resize smaller ----
    io.Resize(2);
    auto shrunk = io.ReadBytes(0, 10);
    assert(shrunk.size() == 2);
    assert(shrunk[0] == 'A');
    assert(shrunk[1] == 'B');

    // ---- 7️⃣ Close and reopen READ-only ----
    io.CloseFile();
    io.OpenFile(testFile, FileIOHandler::FileModes::READ);

    bool resizeFailed = false;
    try {
        io.Resize(5);
    } catch (const FileReadOnlyException&) {
        resizeFailed = true;
    }
    assert(resizeFailed);

    // ---- 7️⃣ Close and reopen WRITE ----
    io.CloseFile();
    io.OpenFile(testFile, FileIOHandler::FileModes::READ_WRITE);

    resizeFailed = false;
    try {
        io.Resize(5);
    } catch (const FileReadOnlyException&) {
        resizeFailed = true;
    }
    assert(!resizeFailed);

    // ---- Cleanup ----
    std::filesystem::remove(testFile);

    std::cout << "✅ FileIOHandler tests passed\n";
}
int main (int argc, char* argv[]) {
    TestFileIOHandler();
};
