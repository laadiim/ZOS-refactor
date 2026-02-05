cmake_minimum_required(VERSION 4.1)
project(ZOS)

set(CMAKE_CXX_STANDARD 17)

add_executable(ZOS_refactor
        main.cpp
        helpers/FileIOHandler.cpp
        helpers/FileIOHandler.h
        helpers/FileIOExceptions.h
        helpers/IntParser.h
        helpers/IntParser.cpp
        include/Filesystem.h
        include/INode.h
        src/INode.cpp
        helpers/ChildNodeNameIdPair.h
        include/Superblock.h
        src/Superblock.cpp
        include/Bitmap.h
        src/Bitmap.cpp
        helpers/SizeParser.h
        src/Filesystem.cpp
        helpers/StringHelpers.h
        helpers/FilesystemExceptions.h
        include/FilesystemInterface.h
        src/FilesystemInterface.cpp
        include/Shell.h
        src/Shell.cpp
)
