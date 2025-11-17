#pragma once

#include "iubpatch/api.h"
#include "iubpatch/errors.h"
#include "iubpatch/io.h"
#include "iubpatch/options.h"
#include <cstdint>
#include <vector>
#include <memory>
#include <string>

namespace iubpatch {

using Byte = std::uint8_t;
using Bytes = std::vector<Byte>;

enum class Format {
    Unknown,
    IPS,
    UPS,
    BPS
};

struct IUBPATCH_API PatchMetadata {
    Format format = Format::Unknown;
    std::size_t src_size = 0;
    std::size_t target_size = 0;
    std::uint32_t source_checksum = 0;
    std::uint32_t target_checksum = 0;
    bool has_checksums = false;
    
    PatchMetadata() = default;
};

class IUBPATCH_API Patch {
public:
    virtual ~Patch() = default;
    
    virtual Format get_format() const noexcept = 0;
    
    virtual Result<PatchMetadata> get_metadata() const = 0;
    
    virtual Result<Bytes> apply(const Bytes& source, const PatchOptions& options = {}) const = 0;
    
    virtual Result<void> apply_to_file(
        const std::string& source_path,
        const std::string& output_path,
        const PatchOptions& options = {}
    ) const = 0;
    
    virtual Result<void> validate() const = 0;
    
    virtual const char* format_name() const noexcept = 0;
    
protected:
    Patch() = default;
};

IUBPATCH_API Result<std::unique_ptr<Patch>> load_patch(const std::string& patch_path);

IUBPATCH_API Result<std::unique_ptr<Patch>> load_patch_from_memory(const Bytes& patch_data);

IUBPATCH_API Result<Format> detect_format(const std::string& patch_path);

IUBPATCH_API Result<Format> detect_format_from_memory(const Bytes& patch_data);

IUBPATCH_API const char* format_to_string(Format format) noexcept;

} // namespace iubpatch
