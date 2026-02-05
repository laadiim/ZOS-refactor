//
// Created by laadim on 18.01.26.
//
#include <cassert>
#include <iostream>
#include <filesystem>

#include "helpers/FileIOHandler.h"
#include "helpers/FileIOExceptions.h"
#include "helpers/IntParser.h"
#include "include/Bitmap.h"
#include "include/Filesystem.h"
#include "include/FilesystemInterface.h"
#include "include/INode.h"
#include "include/Shell.h"
#include "include/Superblock.h"

int main (int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_image>" << std::endl;
        return 1;
    }
    auto fs = FilesystemInterface(argv[1]);
    Shell sh(fs);
    sh.Run();
    return 0;
}
