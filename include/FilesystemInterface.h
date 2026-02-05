//
// Created by laadim on 05.02.26.
//

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "Filesystem.h"

/**
 * @class FilesystemInterface
 * @brief Command-line interface layer for the filesystem.
 *
 * FilesystemInterface acts as an adapter between textual user commands
 * (e.g. "ls", "cp a b") and the Filesystem API.
 *
 * Responsibilities:
 *  - parsing user input
 *  - validating command parameters
 *  - dispatching commands to filesystem methods
 *  - returning structured results
 *
 * This class does NOT:
 *  - perform low-level filesystem logic
 *  - access blocks or inodes directly
 *  - print output to stdout
 *
 * All user interaction is handled by a higher-level shell.
 */
class FilesystemInterface {
public:
    /**
     * @brief Construct the filesystem interface.
     *
     * Initializes the underlying Filesystem instance bound
     * to the given image file and registers all supported
     * shell commands.
     *
     * @param path Path to the filesystem image file.
     */
    explicit FilesystemInterface(std::string path);

    /**
     * @brief Destructor.
     *
     * Flushes filesystem metadata and releases resources.
     */
    ~FilesystemInterface();

    /**
     * @brief Execute a user command.
     *
     * Parses the command string, extracts the command name
     * and parameters, and dispatches execution to the
     * corresponding command handler.
     *
     * @param command Raw command string entered by the user.
     *
     * @return Pair consisting of:
     *  - first  → command output (human-readable)
     *  - second → optional status or error message
     */
    std::pair<std::string, std::string>
    Execute(const std::string& command);

private:
    /** Underlying filesystem instance. */
    std::unique_ptr<Filesystem> filesystem;

    /** Path to the filesystem image file. */
    std::string imagePath;

    /**
     * @brief Mapping of command names to handler functions.
     *
     * Each handler:
     *  - receives parsed command arguments
     *  - returns a result string
     */
    std::map<
        std::string,
        std::function<std::string(
            const std::vector<std::string>&
        )>
    > commandMap;

    // =====================================================
    // Parsing helpers
    // =====================================================

    /**
     * @brief Extract command keyword from input.
     *
     * Example:
     *   Input:  "ls /home"
     *   Output: "ls"
     *
     * @param command Full command string.
     * @return Command keyword.
     */
    std::string ParseCommand(const std::string& command);

    /**
     * @brief Extract command parameters from input.
     *
     * Example:
     *   Input:  "cp a b"
     *   Output: {"a", "b"}
     *
     * @param command Full command string.
     * @return Vector of command parameters.
     */
    std::vector<std::string> ParseParams(const std::string& command);

    // =====================================================
    // Command handlers
    // =====================================================

    /** @brief Copy a file (cp s1 s2). */
    std::string cmd_cp(const std::vector<std::string>& args);

    /** @brief Move or rename a file or directory (mv s1 s2). */
    std::string cmd_mv(const std::vector<std::string>& args);

    /** @brief Remove a file or directory (rm s1). */
    std::string cmd_rm(const std::vector<std::string>& args);

    /** @brief Create a directory (mkdir a1). */
    std::string cmd_mkdir(const std::vector<std::string>& args);

    /** @brief Remove an empty directory (rmdir a1). */
    std::string cmd_rmdir(const std::vector<std::string>& args);

    /** @brief List directory contents (ls [path]). */
    std::string cmd_ls(const std::vector<std::string>& args);

    /** @brief Display file contents (cat s1). */
    std::string cmd_cat(const std::vector<std::string>& args);

    /** @brief Change current working directory (cd a1). */
    std::string cmd_cd(const std::vector<std::string>& args);

    /** @brief Print current working directory (pwd). */
    std::string cmd_pwd(const std::vector<std::string>& args);

    /** @brief Display metadata for a node (info path). */
    std::string cmd_info(const std::vector<std::string>& args);

    /** @brief Display filesystem statistics (statfs). */
    std::string cmd_statfs(const std::vector<std::string>& args);

    /** @brief Copy file from host system into filesystem (incp src dst). */
    std::string cmd_incp(const std::vector<std::string>& args);

    /** @brief Copy file from filesystem to host system (outcp src dst). */
    std::string cmd_outcp(const std::vector<std::string>& args);

    /** @brief Execute commands from a script file (load file). */
    std::string cmd_load(const std::vector<std::string>& args);

    /** @brief Format the filesystem image (format 600MB). */
    std::string cmd_format(const std::vector<std::string>& args);

    /** @brief Terminate the shell session (exit). */
    std::string cmd_exit(const std::vector<std::string>& args);

    /** @brief Create a hard link (ln existing newpath). */
    std::string cmd_ln(const std::vector<std::string>& args);

    // =====================================================
    // Command registration
    // =====================================================

    /**
     * @brief Register all supported shell commands.
     *
     * Maps textual command names to handler methods
     * using lambdas bound to this instance.
     */
    void RegisterCommands() {
        commandMap["cp"]     = [this](auto& args) { return cmd_cp(args); };
        commandMap["mv"]     = [this](auto& args) { return cmd_mv(args); };
        commandMap["rm"]     = [this](auto& args) { return cmd_rm(args); };

        commandMap["mkdir"]  = [this](auto& args) { return cmd_mkdir(args); };
        commandMap["rmdir"]  = [this](auto& args) { return cmd_rmdir(args); };
        commandMap["ls"]     = [this](auto& args) { return cmd_ls(args); };

        commandMap["cat"]    = [this](auto& args) { return cmd_cat(args); };
        commandMap["cd"]     = [this](auto& args) { return cmd_cd(args); };
        commandMap["pwd"]    = [this](auto& args) { return cmd_pwd(args); };

        commandMap["info"]   = [this](auto& args) { return cmd_info(args); };
        commandMap["statfs"] = [this](auto& args) { return cmd_statfs(args); };

        commandMap["incp"]   = [this](auto& args) { return cmd_incp(args); };
        commandMap["outcp"]  = [this](auto& args) { return cmd_outcp(args); };

        commandMap["load"]   = [this](auto& args) { return cmd_load(args); };
        commandMap["format"] = [this](auto& args) { return cmd_format(args); };
        commandMap["exit"]   = [this](auto& args) { return cmd_exit(args); };

        commandMap["ln"]     = [this](auto& args) { return cmd_ln(args); };
    }
};