//
// Created by laadim on 05.02.26.
//

#pragma once
#include "FilesystemInterface.h"

/**
* @class Shell
* @brief Interactive command-line shell for the filesystem.
*
* The Shell class represents the top-level user interface of the system.
* It is responsible for:
*
*  - displaying a command prompt
*  - reading user input from standard input
*  - forwarding commands to FilesystemInterface
*  - printing command results to standard output
*
* IMPORTANT:
*  - The shell contains NO filesystem logic
*  - It does NOT parse paths or manipulate inodes
*  - All filesystem operations are delegated to FilesystemInterface
*
* This separation keeps user interaction independent of filesystem logic.
*/
class Shell {
public:
    /**
     * @brief Constructs the shell.
     *
     * The shell operates on an existing FilesystemInterface instance.
     * The interface is expected to remain valid for the lifetime
     * of the shell.
     *
     * @param fs Reference to the filesystem command interface.
     */
    explicit Shell(FilesystemInterface& fs);

    /**
     * @brief Starts the interactive shell loop.
     *
     * Behavior:
     *  - prints a prompt (`> `)
     *  - reads a line from standard input
     *  - ignores empty or whitespace-only input
     *  - executes the command via FilesystemInterface
     *  - prints results or error messages
     *
     * The loop terminates when:
     *  - the user enters the `exit` command
     *  - end-of-file (EOF) is reached on standard input
     */
    void Run();

private:
    /**
     * @brief Reference to the filesystem command dispatcher.
     *
     * This object:
     *  - parses commands
     *  - resolves arguments
     *  - executes filesystem operations
     *
     * The shell only forwards user input and displays results.
     */
    FilesystemInterface& fs;
};