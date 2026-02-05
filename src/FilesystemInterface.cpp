//
// Created by laadim on 05.02.26.
//
#include "../include/FilesystemInterface.h"

#include <sstream>

#include "../helpers/FileIOExceptions.h"
#include "../helpers/FilesystemExceptions.h"
#include "../helpers/SizeParser.h"

FilesystemInterface::FilesystemInterface(std::string path) {
    this->filesystem = std::make_unique<Filesystem>(path);
    this->imagePath = path;
    this->RegisterCommands();
}

FilesystemInterface::~FilesystemInterface() = default;

std::pair<std::string, std::string>
FilesystemInterface::Execute(const std::string &command) {
    std::string cmd = ParseCommand(command);
    auto args = ParseParams(command);

    if (!this->filesystem->Formated() && !(cmd == "format" || cmd == "load" || cmd == "exit")) {
        return {"", "Filesystem not formated"};
    }

    std::string msg;
    try {
        msg = commandMap[cmd](args);
        auto cwd = this->cmd_pwd({});
        return {cwd, msg};
    }
    catch (std::exception& e) {
        auto cwd = this->cmd_pwd({});
        msg = e.what();
        if (msg == "bad_function_call") {
            msg = "Unknown command";
        }
        return { cwd, std::string("Error: ") + msg };
    }
}

std::string FilesystemInterface::ParseCommand(const std::string &command) {
    std::istringstream iss(command);
    std::string out;
    iss >> out;
    return out;
}

std::vector<std::string> FilesystemInterface::ParseParams(const std::string &command) {
    std::istringstream iss(command);
    std::vector<std::string> params;
    std::string token;

    // Skip command name
    iss >> token;

    while (iss >> token) {
        params.push_back(token);
    }
    return params;
}

std::string FilesystemInterface::cmd_cp(const std::vector<std::string> &args) {
    if (args.size() != 2) {
        return "Usage: cp <src> <dst>";
    }
    this->filesystem->CopyFile(args[0], args[1]);

    return "Copied successfully";
}

std::string FilesystemInterface::cmd_mv(const std::vector<std::string> &args) {
    if (args.size() != 2) {
        return "Usage: mv <src> <dst>";
    }

    filesystem->MoveFile(args[0], args[1]);
    return "Moved successfully";
}

std::string FilesystemInterface::cmd_rm(const std::vector<std::string> &args) {
    if (args.size() != 1) {
        return "Usage: rm <file>";
    }

    filesystem->RemoveFile(args[0]);
    return "File removed";
}

std::string FilesystemInterface::cmd_mkdir(const std::vector<std::string> &args) {
    if (args.size() != 1) {
        return "Usage: mkdir <dir>";
    }

    filesystem->CreateDirectory(args[0]);
    return "Directory created";
}

std::string FilesystemInterface::cmd_rmdir(const std::vector<std::string> &args) {
    if (args.size() != 1) {
        return "Usage: rmdir <dir>";
    }

    filesystem->RemoveDirectory(args[0]);
    return "Directory removed";
}

std::string FilesystemInterface::cmd_ls(const std::vector<std::string> &args) {
    std::string path = args.empty() ? "." : args[0];

    auto entries = filesystem->GetSubdirectories(path);
    std::ostringstream out;

    for (const auto& [name, isDir] : entries) {
        out << (isDir ? "[D] " : "[F] ") << name << "\n";
    }

    return out.str();
}

std::string FilesystemInterface::cmd_cat(const std::vector<std::string> &args) {
    if (args.size() != 1) {
        return "Usage: cat <file>";
    }

    auto data = filesystem->ReadFile(args[0]);
    return std::string(data.begin(), data.end());
}

std::string FilesystemInterface::cmd_cd(const std::vector<std::string> &args) {
    if (args.size() != 1) {
        return "Usage: cd <dir>";
    }

    filesystem->ChangeActiveDirectory(args[0]);
    return "";
}

std::string FilesystemInterface::cmd_pwd(const std::vector<std::string> &) {
    auto parts = filesystem->GetCurrentPath();

    std::ostringstream out;
    out << "/";

    for (size_t i = 0; i < parts.size(); ++i) {
        out << parts[i];
        if (i + 1 < parts.size()) out << "/";
    }

    return out.str();
}

std::string FilesystemInterface::cmd_info(const std::vector<std::string> &args) {
    if (args.size() != 1) {
        return "Usage: info <path>";
    }

    return filesystem->GetNodeInfo(args[0]);
}

std::string FilesystemInterface::cmd_statfs(const std::vector<std::string> &) {
    return filesystem->GetFilesystemStats();
}

std::string FilesystemInterface::cmd_incp(const std::vector<std::string> &args) {
    if (args.size() != 2) {
        return "Usage: incp <host_file> <fs_path>";
    }

    std::ifstream in(args[0], std::ios::binary);
    if (!in) return "Could not open host file";

    std::vector<char> data(
        (std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>()
    );

    filesystem->WriteFile(args[1], data);
    return "Imported file";
}

std::string FilesystemInterface::cmd_outcp(const std::vector<std::string> &args) {
    if (args.size() != 2) {
        return "Usage: outcp <fs_file> <host_path>";
    }

    auto data = filesystem->ReadFile(args[0]);

    std::ofstream out(args[1], std::ios::binary);
    if (!out) return "Could not create host file";

    out.write(data.data(), data.size());
    return "Exported file";
}

std::string FilesystemInterface::cmd_load(const std::vector<std::string> &args) {
    if (args.size() != 1) {
        return "Usage: load <script_file>";
    }

    std::ifstream in(args[0]);
    if (!in.is_open()) {
        return "FILE NOT FOUND";
    }

    std::string line;
    while (std::getline(in, line)) {
        // ignore empty lines (optional)
        if (line.empty()) continue;

        // optional: ignore comment lines
        // if (!line.empty() && line[0] == '#') continue;

        auto [cwd, result] = Execute(line);

        if (result == "exit") {
            break;
        }

        if (result.rfind("Error", 0) == 0 || result == "Unknown command") {
            return result;
        }
    }

    return "OK";
}

std::string FilesystemInterface::cmd_format(const std::vector<std::string> &args) {
    if (args.size() != 1) {
        return "Usage: format <size_bytes>";
    }

    uint64_t size = 0;
    ParseSize(args[0], size);
    filesystem->Format(size);

    return "Filesystem formatted";
}

std::string FilesystemInterface::cmd_exit(const std::vector<std::string> &) {
    return "exit";
}

std::string FilesystemInterface::cmd_ln(const std::vector<std::string> &args) {
    if (args.size() != 2) {
        return "Usage: ln <target> <link>";
    }

    filesystem->LinkFile(args[0], args[1]);
    return "Link created";
}
