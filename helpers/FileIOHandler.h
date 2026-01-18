//
// Created by laadim on 18.01.26.
//

#ifndef ZOS_REFACTOR_FILEIOHANDLER_H
#define ZOS_REFACTOR_FILEIOHANDLER_H
#include <cstdint>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

/**
 * @brief Stream based IO handler
 *
 * Class handling IO operations over a file.
 * Can create and resize files.
 *
 * @author laadim
 */
class FileIOHandler {
    public:
    enum class FileModes {
        READ, READ_WRITE
    };

    /**
     * @brief IO handler constructor
     */
    FileIOHandler() = default;

    /**
     * @brief IO handler destructor
     *
     * Flushes and closes stream.
     */
    ~FileIOHandler();

    /**
     * @brief Open a file
     *
     * Open a stream over the file.
     * If in write mode, creates the file if it does not exist
     *
     * @param fileName path to file
     * @param mode mode to open stream in
     *
     * @throw FileDoesNotExistException File was not found
     * @throw CouldNotOpenFile Error occurred while opening the file
     *
     * @return True if file was opened
     */
    void OpenFile(const std::string &fileName, FileModes mode);

    /**
     * @brief Flush and close
     */
    void CloseFile() const;


    void EnsureWritable() const;

    /**
     * @brief Read a vector of bytes
     *
     * @param offset Start of bytes to read
     * @param size Number of bytes
     * @return Vector of bytes read
     */
    std::vector<char> ReadBytes(uint64_t offset, uint64_t size) const;

    /**
     * @brief Write data to file
     *
     * @param offset Start of bytes to write
     * @param data Data to write
     * @return Number of bytes written
     */
    uint64_t WriteBytes(uint64_t offset, const std::vector<char> &data) const;

    /**
     * @brief Flush stream
     */
    void Flush() const;

    /**
     * @brief Resizes open file
     *
     * @param newSize Size to resize to
     * @return New size
     */
    uint64_t Resize(uint64_t newSize) const;

    /**
     * @brief Checks if stream is open
     *
     * @return File is open
     */
    bool IsOpen() const;

    private:
    /// Name of opened file
    std::string fileName;

    /// File stream
    std::unique_ptr<std::fstream> stream;

    /// Stream mode
    FileModes mode;
};


#endif //ZOS_REFACTOR_FILEIOHANDLER_H