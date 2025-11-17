#include "iubpatch/io.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace iubpatch {

// Windows implementation of mmap (kinda)
class MappedFileReader::Impl {
public:
    HANDLE file_handle = INVALID_HANDLE_VALUE;
    HANDLE mapping_handle = nullptr;
    void* mapped_data = nullptr;
    std::size_t file_size = 0;
    
    ~Impl() {
        if (mapped_data) {
            UnmapViewOfFile(mapped_data);
        }
        if (mapping_handle) {
            CloseHandle(mapping_handle);
        }
        if (file_handle != INVALID_HANDLE_VALUE) {
            CloseHandle(file_handle);
        }
    }
    
    static Result<std::unique_ptr<Impl>> open(const std::string& path) {
        auto impl = std::make_unique<Impl>();
        
        impl->file_handle = CreateFileA(
            path.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );
        
        if (impl->file_handle == INVALID_HANDLE_VALUE) {
            return ErrorInfo{ErrorCode::FileNotFound, "Cannot open file: " + path};
        }
        
        LARGE_INTEGER size;
        if (!GetFileSizeEx(impl->file_handle, &size)) {
            return ErrorInfo{ErrorCode::FileReadError, "Cannot get file size: " + path};
        }
        impl->file_size = static_cast<std::size_t>(size.QuadPart);
        
        if (impl->file_size > 0) {
            impl->mapping_handle = CreateFileMappingA(
                impl->file_handle,
                nullptr,
                PAGE_READONLY,
                0, 0,
                nullptr
            );
            
            if (!impl->mapping_handle) {
                return ErrorInfo{ErrorCode::MmapFailed, "Cannot create file mapping: " + path};
            }
            
            impl->mapped_data = MapViewOfFile(
                impl->mapping_handle,
                FILE_MAP_READ,
                0, 0,
                impl->file_size
            );
            
            if (!impl->mapped_data) {
                return ErrorInfo{ErrorCode::MmapFailed, "Cannot map view of file: " + path};
            }
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

#endif // _WIN32
