#include "iubpatch/io.h"
#include <fstream>
#include <cstring>

#if defined(__unix__) || defined(__APPLE__)
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

namespace iubpatch {

const char* PatchErrorCategory::name() const noexcept {
    return "iubpatch";
}

std::string PatchErrorCategory::message(int ev) const {
    switch (static_cast<ErrorCode>(ev)) {
        case ErrorCode::Success:
            return "Success";
        case ErrorCode::FileNotFound:
            return "File not found";
        case ErrorCode::FileReadError:
            return "File read error";
        case ErrorCode::FileWriteError:
            return "File write error";
        case ErrorCode::FileAccessDenied:
            return "File access denied";
        case ErrorCode::FileTooLarge:
            return "File too large";
        case ErrorCode::InvalidPatchFormat:
            return "Invalid patch format";
        case ErrorCode::UnsupportedPatchVersion:
            return "Unsupported patch version";
        case ErrorCode::CorruptedPatchData:
            return "Corrupted patch data";
        case ErrorCode::InvalidPatchHeader:
            return "Invalid patch header";
        case ErrorCode::InvalidSourceFile:
            return "Invalid source file";
        case ErrorCode::SourceSizeMismatch:
            return "Source size mismatch";
        case ErrorCode::ChecksumMismatch:
            return "Checksum mismatch";
        case ErrorCode::InvalidPatchOffset:
            return "Invalid patch offset";
        case ErrorCode::PatchTooLarge:
            return "Patch too large";
        case ErrorCode::OutOfMemory:
            return "Out of memory";
        case ErrorCode::MmapFailed:
            return "Memory mapping failed";
        case ErrorCode::InvalidArgument:
            return "Invalid argument";
        case ErrorCode::UnknownError:
        default:
            return "Unknown error";
    }
}

const std::error_category& patch_category() {
    static PatchErrorCategory instance;
    return instance;
}

std::error_code make_error_code(ErrorCode e) {
    return {static_cast<int>(e), patch_category()};
}

class BufferedFileReader::Impl {
public:
    Bytes data;
    std::size_t file_size = 0;
    
    static Result<std::unique_ptr<BufferedFileReader>> open(const std::string& path) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) {
            return ErrorInfo{ErrorCode::FileNotFound, "Cannot open file: " + path};
        }
        
        auto impl = std::make_unique<Impl>();
        impl->file_size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        impl->data.resize(impl->file_size);
        if (!file.read(reinterpret_cast<char*>(impl->data.data()), impl->file_size)) {
            return ErrorInfo{ErrorCode::FileReadError, "Cannot read file: " + path};
        }
        
        auto reader = std::unique_ptr<BufferedFileReader>(new BufferedFileReader());
        reader->impl_ = std::move(impl);
        return reader;
    }
};

BufferedFileReader::BufferedFileReader() : impl_(std::make_unique<Impl>()) {}
BufferedFileReader::~BufferedFileReader() = default;

Result<std::unique_ptr<BufferedFileReader>> BufferedFileReader::open(const std::string& path) {
    return Impl::open(path);
}

Result<Bytes> BufferedFileReader::read_all() {
    return impl_->data;
}

Result<std::size_t> BufferedFileReader::size() const {
    return impl_->file_size;
}

Result<Bytes> BufferedFileReader::read_range(std::size_t offset, std::size_t length) {
    if (offset + length > impl_->file_size) {
        return ErrorInfo{ErrorCode::InvalidArgument, "Read range out of bounds"};
    }
    Bytes result(impl_->data.begin() + offset, impl_->data.begin() + offset + length);
    return result;
}

const Byte* BufferedFileReader::data() const {
    return impl_->data.data();
}

class BufferedFileWriter::Impl {
public:
    std::ofstream file;
    
    static Result<std::unique_ptr<BufferedFileWriter>> create(const std::string& path) {
        auto impl = std::make_unique<Impl>();
        impl->file.open(path, std::ios::binary | std::ios::trunc);
        if (!impl->file) {
            return ErrorInfo{ErrorCode::FileWriteError, "Cannot create file: " + path};
        }
        
        auto writer = std::unique_ptr<BufferedFileWriter>(new BufferedFileWriter());
        writer->impl_ = std::move(impl);
        return writer;
    }
};

BufferedFileWriter::BufferedFileWriter() : impl_(std::make_unique<Impl>()) {}
BufferedFileWriter::~BufferedFileWriter() = default;

Result<std::unique_ptr<BufferedFileWriter>> BufferedFileWriter::create(const std::string& path) {
    return Impl::create(path);
}

Result<void> BufferedFileWriter::write(std::span<const Byte> data) {
    if (!impl_->file.write(reinterpret_cast<const char*>(data.data()), data.size())) {
        return ErrorInfo{ErrorCode::FileWriteError, "Write failed"};
    }
    return Result<void>{};
}

Result<void> BufferedFileWriter::write_at(std::size_t offset, std::span<const Byte> data) {
    impl_->file.seekp(offset);
    return write(data);
}

Result<void> BufferedFileWriter::flush() {
    impl_->file.flush();
    return Result<void>{};
}

Result<std::unique_ptr<FileReader>> open_file_reader(const std::string& path, bool prefer_mmap) {
    #ifdef IUB_ENABLE_MMAP
    if (prefer_mmap) {
        auto mmap_result = MappedFileReader::open(path);
        if (mmap_result) {
            return std::unique_ptr<FileReader>(mmap_result.value().release());
        }
    }
    #endif
    
    auto result = BufferedFileReader::open(path);
    if (!result) {
        return result.error();
    }
    return std::unique_ptr<FileReader>(result.value().release());
}

Result<std::unique_ptr<FileWriter>> create_file_writer(const std::string& path) {
    auto result = BufferedFileWriter::create(path);
    if (!result) {
        return result.error();
    }
    return std::unique_ptr<FileWriter>(result.value().release());
}

Result<Bytes> read_file(const std::string& path) {
    auto reader_result = open_file_reader(path, false);
    if (!reader_result) {
        return reader_result.error();
    }
    return reader_result.value()->read_all();
}

Result<void> write_file(const std::string& path, std::span<const Byte> data) {
    auto writer_result = create_file_writer(path);
    if (!writer_result) {
        return writer_result.error();
    }
    auto write_result = writer_result.value()->write(data);
    if (!write_result) {
        return write_result;
    }
    return writer_result.value()->flush();
}

} // namespace iubpatch
