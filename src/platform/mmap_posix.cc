#include "iubpatch/io.h"

#if defined(__unix__) || defined(__APPLE__)

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace iubpatch {

// POSIX implementation of MappedFileReader::Impl
class MappedFileReader::Impl {
public:
    int fd = -1;
    void* mapped_data = nullptr;
    std::size_t file_size = 0;
    
    ~Impl() {
        if (mapped_data && mapped_data != MAP_FAILED) {
            munmap(mapped_data, file_size);
        }
        if (fd >= 0) {
            close(fd);
        }
    }
    
    static Result<std::unique_ptr<Impl>> open(const std::string& path) {
        auto impl = std::make_unique<Impl>();
        
        impl->fd = ::open(path.c_str(), O_RDONLY);
        if (impl->fd < 0) {
            return ErrorInfo{ErrorCode::FileNotFound, "Cannot open file: " + path};
        }
        
        struct stat st;
        if (fstat(impl->fd, &st) < 0) {
            return ErrorInfo{ErrorCode::FileReadError, "Cannot stat file: " + path};
        }
        impl->file_size = st.st_size;
        
        if (impl->file_size > 0) {
            impl->mapped_data = mmap(nullptr, impl->file_size, PROT_READ, 
                                     MAP_PRIVATE, impl->fd, 0);
            if (impl->mapped_data == MAP_FAILED) {
                return ErrorInfo{ErrorCode::MmapFailed, "Memory mapping failed: " + path};
            }
            
            madvise(impl->mapped_data, impl->file_size, MADV_SEQUENTIAL);
        }
        
        return impl;
    }
};

MappedFileReader::MappedFileReader() : impl_(nullptr) {}
MappedFileReader::~MappedFileReader() = default;


Result<std::unique_ptr<MappedFileReader>> MappedFileReader::open(const std::string& path) {
    #ifdef IUB_ENABLE_MMAP
        auto impl_result = Impl::open(path);
        if (!impl_result) {
            return impl_result.error();
        }
        
        auto reader = std::unique_ptr<MappedFileReader>(new MappedFileReader());
        reader->impl_ = std::move(impl_result.value());
        return reader;
    #else
        return ErrorInfo{ErrorCode::MmapFailed, "Memory mapping disabled"};
    #endif
}

Result<Bytes> MappedFileReader::read_all() {
    if (!impl_ || !impl_->mapped_data) {
        return ErrorInfo{ErrorCode::MmapFailed, "No mapped data"};
    }
    Bytes result(static_cast<const Byte*>(impl_->mapped_data),
                 static_cast<const Byte*>(impl_->mapped_data) + impl_->file_size);
    return result;
}

Result<std::size_t> MappedFileReader::size() const {
    if (!impl_) {
        return ErrorInfo{ErrorCode::MmapFailed, "No mapped data"};
    }
    return impl_->file_size;
}

Result<Bytes> MappedFileReader::read_range(std::size_t offset, std::size_t length) {
    if (!impl_ || !impl_->mapped_data) {
        return ErrorInfo{ErrorCode::MmapFailed, "No mapped data"};
    }
    if (offset + length > impl_->file_size) {
        return ErrorInfo{ErrorCode::InvalidArgument, "Read range out of bounds"};
    }
    const Byte* start = static_cast<const Byte*>(impl_->mapped_data) + offset;
    Bytes result(start, start + length);
    return result;
}

const Byte* MappedFileReader::data() const {
    return impl_ ? static_cast<const Byte*>(impl_->mapped_data) : nullptr;
}

} // namespace iubpatch

#endif // defined(__unix__) || defined(__APPLE__)
