//
// Created by laadim on 18.01.26.
//

#ifndef ZOS_REFACTOR_EXCEPTIONS_H
#define ZOS_REFACTOR_EXCEPTIONS_H
#include <exception>
#include <stdexcept>
#include <string>

class FileDoesNotExistException : public std::runtime_error {
    public:
    explicit FileDoesNotExistException(const std::string &msg) : std::runtime_error{msg} {};
};

class CouldNotOpenFileException : public std::runtime_error {
    public:
    explicit CouldNotOpenFileException(const std::string &msg) : std::runtime_error{msg} {};
};

class FileNotOpenException : public std::runtime_error {
    public:
    explicit FileNotOpenException(const std::string &msg) : std::runtime_error{msg} {};
};

class FileReadOnlyException : public std::runtime_error {
    public:
    explicit FileReadOnlyException(const std::string &msg) : std::runtime_error{msg} {};
};

class FileReadException : public std::runtime_error {
    public:
    explicit FileReadException(const std::string &msg) : std::runtime_error{msg} {};
};

class FileWriteException : public std::runtime_error {
    public:
    explicit FileWriteException(const std::string &msg) : std::runtime_error{msg} {};
};

#endif //ZOS_REFACTOR_EXCEPTIONS_H