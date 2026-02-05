//
// Created by laadim on 05.02.26.
//
#include "../include/Shell.h"

#include <algorithm>
#include <iostream>

// =====================================================
// Construction
// =====================================================

/**
 * @brief Constructs the shell.
 *
 * @param fs Reference to filesystem interface used to execute commands.
 */
Shell::Shell(FilesystemInterface& fs)
    : fs(fs) {}

// =====================================================
// Main loop
// =====================================================

/**
 * @brief Runs the interactive shell loop.
 *
 * Responsibilities:
 *  - display prompt
 *  - read user input
 *  - dispatch commands to FilesystemInterface
 *  - print results or errors
 *
 * The shell is the ONLY component responsible for:
 *  - standard input
 *  - standard output
 *  - user interaction
 */
void Shell::Run() {
    std::pair<std::string, std::string> result;
    result.first = "/";
    while (true) {
        // Display prompt
        std::cout << result.first << " > ";

        // Read input line
        std::string line;
        if (!std::getline(std::cin, line)) {
            // End-of-file (Ctrl+D / Ctrl+Z)
            break;
        }

        // Skip empty or whitespace-only lines
        if (std::all_of(line.begin(), line.end(), ::isspace)) {
            continue;
        }

        // Execute command via interface
        result = fs.Execute(line);

        // Handle shell exit
        if (result.second == "exit") {
            break;
        }

        // Print result
        std::cout << result.second << std::endl;
    }
}
