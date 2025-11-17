#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <span>

#include "iubpatch/api.h"
#include "iubpatch/errors.h"

namespace iubpatch {

using Byte = std::uint8_t;
using Bytes = std::vector<Byte>;

class IUBPATCH_API FileReader {
public:
    virtual ~FileReader() = default;

    virtual Result<Bytes> read_all() = 0;
    
    virtual Result<std::size_t> size() const = 0;
    
    virtual Result<Bytes> read_range(std::size_t offset, std::size_t length) = 0;
    
    virtual const Byte* data() const = 0;
    
    virtual bool is_mapped() const noexcept = 0;
};

class IUBPATCH_API FileWriter {
public:
    virtual ~FileWriter() = default;
    
    virtual Result<void> write(std::span<const Byte> data) = 0;
    
    virtual Result<void> write_at(std::size_t offset, std::span<const Byte> data) = 0;
    
    virtual Result<void> flush() = 0;
};

// use mmap if possible for reading, otherwise falls back to BufferedFileReader
// still no idea if this is even needed since most roms & patches are small anyways
class IUBPATCH_API MappedFileReader : public FileReader {
public:
    static Result<std::unique_ptr<MappedFileReader>> open(const std::string& path);

    ~MappedFileReader() override;
    
    Result<Bytes> read_all() override;

    Result<std::size_t> size() const override;

    Result<Bytes> read_range(std::size_t offset, std::size_t length) override;

    const Byte* data() const override;

    bool is_mapped() const noexcept override {
        return true;
    }
    
private:
    MappedFileReader();
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// usual buffered file reader incase mmap does not work or is not preferred
class IUBPATCH_API BufferedFileReader : public FileReader {
public:
    static Result<std::unique_ptr<BufferedFileReader>> open(const std::string& path);

    ~BufferedFileReader() override;
    
    Result<Bytes> read_all() override;

    Result<std::size_t> size() const override;

    Result<Bytes> read_range(std::size_t offset, std::size_t length) override;

    const Byte* data() const override;

    bool is_mapped() const noexcept override {
        return false;
    }
    
private:
    BufferedFileReader();
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// buffered file writer
class IUBPATCH_API BufferedFileWriter : public FileWriter {
public:
    static Result<std::unique_ptr<BufferedFileWriter>> create(const std::string& path);

    ~BufferedFileWriter() override;
    
    Result<void> write(std::span<const Byte> data) override;

    Result<void> write_at(std::size_t offset, std::span<const Byte> data) override;
    
    Result<void> flush() override;
    
private:
    BufferedFileWriter();
    class Impl;
    std::unique_ptr<Impl> impl_;
};

IUBPATCH_API Result<std::unique_ptr<FileReader>> open_file_reader(const std::string& path, bool prefer_mmap = true);

IUBPATCH_API Result<std::unique_ptr<FileWriter>> create_file_writer(const std::string& path);

IUBPATCH_API Result<Bytes> read_file(const std::string& path);

IUBPATCH_API Result<void> write_file(const std::string& path, std::span<const Byte> data);

} // namespace iubpatch
