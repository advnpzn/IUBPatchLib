#include "iubpatch/formats/ups.h"
#include "iubpatch/io.h"
#include "iubpatch/crc32.h"
#include <cstdint>
#include <cstring>
#include <algorithm>

namespace iubpatch {

// UPS magic
static constexpr char UPS_MAGIC[] = "UPS1";
static constexpr std::size_t UPS_HEADER_SIZE = 4;

static std::uint64_t decode_variable_len(const Bytes& data, std::size_t& offset) {
    std::uint64_t value = 0;
    std::uint64_t shift = 1;
    
    while (offset < data.size()) {
        std::uint64_t byte = data[offset++];
        value += (byte & 0x7F) * shift;
        if (byte & 0x80) break;
        shift <<= 7;
        value += shift;
    }
    
    return value;
}

class UPSPatch::Impl {
public:
    Bytes patch_data;
    std::size_t src_size = 0;
    std::size_t target_size = 0;
    std::uint32_t src_crc = 0;
    std::uint32_t target_crc = 0;
    std::uint32_t patch_crc = 0;
    
    struct XORBlock {
        std::size_t offset;
        Bytes data;
    };
    std::vector<XORBlock> blocks;
    
    Result<void> parse() {
        blocks.clear();
        
        if (patch_data.size() < UPS_HEADER_SIZE + 12) {
            return ErrorInfo{ErrorCode::InvalidPatchFormat, "UPS patch too small"};
        }
        
        std::size_t offset = UPS_HEADER_SIZE;
        
        src_size = decode_variable_len(patch_data, offset);
        target_size = decode_variable_len(patch_data, offset);
        
        std::size_t file_offset = 0;
        while (offset + 12 < patch_data.size()) {

            std::uint64_t relative_offset = decode_variable_len(patch_data, offset);
            file_offset += relative_offset;
            
            XORBlock block;
            block.offset = file_offset;
            
            while (offset < patch_data.size() - 12) {
                Byte b = patch_data[offset++];
                if (b == 0x00) break;
                block.data.push_back(b);
                file_offset++;
            }
            
            if (!block.data.empty()) {
                blocks.push_back(std::move(block));
            }
        }
        
        if (patch_data.size() >= 12) {
            std::size_t crc_offset = patch_data.size() - 12;
            src_crc = *reinterpret_cast<const std::uint32_t*>(&patch_data[crc_offset]);
            target_crc = *reinterpret_cast<const std::uint32_t*>(&patch_data[crc_offset + 4]);
            patch_crc = *reinterpret_cast<const std::uint32_t*>(&patch_data[crc_offset + 8]);
        }
        
        return Result<void>{};
    }
};

UPSPatch::UPSPatch() : impl_(std::make_unique<Impl>()) {}
UPSPatch::~UPSPatch() = default;

Result<std::unique_ptr<UPSPatch>> UPSPatch::load(const Bytes& patch_data) {

    if (patch_data.size() < UPS_HEADER_SIZE + 12) {
        return ErrorInfo{ErrorCode::InvalidPatchFormat, "UPS patch too small"};
    }
    
    if (std::memcmp(patch_data.data(), UPS_MAGIC, UPS_HEADER_SIZE) != 0) {
        return ErrorInfo{ErrorCode::InvalidPatchHeader, "Invalid UPS header"};
    }
    
    auto patch = std::unique_ptr<UPSPatch>(new UPSPatch());
    patch->impl_->patch_data = patch_data;
    
    auto parse_result = patch->impl_->parse();
    if (!parse_result) {
        return parse_result.error();
    }
    
    return patch;
}

Result<std::unique_ptr<UPSPatch>> UPSPatch::load_from_file(const std::string& path) {
    auto data_result = read_file(path);

    if (!data_result) {
        return data_result.error();
    }

    return load(data_result.value());
}

Result<PatchMetadata> UPSPatch::get_metadata() const {
    PatchMetadata metadata;
    metadata.format = Format::UPS;
    metadata.has_checksums = true;
    metadata.src_size = impl_->src_size;
    metadata.target_size = impl_->target_size;
    metadata.source_checksum = impl_->src_crc;
    metadata.target_checksum = impl_->target_crc;
    
    return metadata;
}

Result<Bytes> UPSPatch::apply(const Bytes& source, const PatchOptions& options) const {

    if (options.verify_checksums) {
        if (source.size() != impl_->src_size) {
            return ErrorInfo{ErrorCode::SourceSizeMismatch, 
                "Source size mismatch: expected " + std::to_string(impl_->src_size) + 
                ", got " + std::to_string(source.size())};
        }
        
        auto src_crc = calc_crc32(source);
        if (src_crc != impl_->src_crc) {
            return ErrorInfo{ErrorCode::ChecksumMismatch, "Source CRC32 mismatch"};
        }
    }
    
    Bytes output;
    std::size_t max_size = std::max(source.size(), impl_->target_size);
    output.resize(max_size);
    
    std::copy(source.begin(), source.end(), output.begin());
    
    for (const auto& block : impl_->blocks) {
        for (std::size_t i = 0; i < block.data.size() && (block.offset + i) < output.size(); ++i) {
            output[block.offset + i] ^= block.data[i];
        }
    }
    
    output.resize(impl_->target_size);
    
    if (options.verify_checksums) {
        auto target_crc = calc_crc32(output);
        if (target_crc != impl_->target_crc) {
            return ErrorInfo{ErrorCode::ChecksumMismatch, "Target CRC32 mismatch"};
        }
    }
    
    return output;
}

Result<void> UPSPatch::apply_to_file(
    const std::string& source_path,
    const std::string& output_path,
    const PatchOptions& options
) const {

    auto source_result = read_file(source_path);
    if (!source_result) {
        return source_result.error();
    }
    
    auto patched_result = apply(source_result.value(), options);
    if (!patched_result) {
        return patched_result.error();
    }
    
    auto& patched_data = patched_result.value();
    return write_file(output_path, std::span<const Byte>(patched_data.data(), patched_data.size()));
}

Result<void> UPSPatch::validate() const {
    if (impl_->patch_data.size() < UPS_HEADER_SIZE + 12) {
        return ErrorInfo{ErrorCode::InvalidPatchFormat, "UPS patch too small"};
    }
    
    if (std::memcmp(impl_->patch_data.data(), UPS_MAGIC, UPS_HEADER_SIZE) != 0) {
        return ErrorInfo{ErrorCode::InvalidPatchHeader, "Invalid UPS header"};
    }
    
    Bytes patch_without_crc(impl_->patch_data.begin(), impl_->patch_data.end() - 4);
    auto calculated_crc = calc_crc32(patch_without_crc);
    
    if (calculated_crc != impl_->patch_crc) {
        return ErrorInfo{ErrorCode::ChecksumMismatch, "Patch CRC32 mismatch"};
    }
    
    return Result<void>{};
}

Result<void> UPSPatch::verify_checksums(const Bytes& source, const Bytes& target) const {
    auto src_crc = calc_crc32(source);
    if (src_crc != impl_->src_crc) {
        return ErrorInfo{ErrorCode::ChecksumMismatch, "Source CRC32 mismatch"};
    }
    
    auto target_crc = calc_crc32(target);
    if (target_crc != impl_->target_crc) {
        return ErrorInfo{ErrorCode::ChecksumMismatch, "Target CRC32 mismatch"};
    }
    
    return Result<void>{};
}

bool is_ups_format(const Bytes& data) {
    return data.size() >= UPS_HEADER_SIZE && 
           std::memcmp(data.data(), UPS_MAGIC, UPS_HEADER_SIZE) == 0;
}

} // namespace iubpatch
