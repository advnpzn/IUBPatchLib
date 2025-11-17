#pragma once

#include "iubpatch/api.h"
#include <cstdint>

namespace iubpatch {

// patch options
struct IUBPATCH_API PatchOptions {
    bool verify_checksums = true;
    bool validate_src_size = true;
    bool allow_size_mismatch = false;
    bool use_mmap = true;
    std::size_t max_mmap_size = 0;
    std::size_t io_buffer_size = 65536; // 64KB
    bool create_backup = false;
    const char* backup_suffix = ".bak";

    PatchOptions() = default;
};

struct IUBPATCH_API CreateOptions {
    enum class Format {
        IPS,
        UPS,
        BPS,
        Auto
    } format = Format::Auto;

    int optimization_level = 2;
    bool include_metadata = true;
    
    CreateOptions() = default;
};

} // namespace iubpatch
