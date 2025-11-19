#include "iubpatch/formats/bps.h"
#include "iubpatch/io.h"
#include "iubpatch/crc32.h"
#include <cstdint>
#include <cstring>
#include <algorithm>

namespace iubpatch {

// BPS magic
static constexpr char BPS_MAGIC[] = "BPS1";
static constexpr std::size_t BPS_HEADER_SIZE = 4;

static std::uint64_t decode_bps_num(const Bytes& data, std::size_t& offset) {
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

class BPSPatch::Impl {
public:
    Bytes patch_data;
    std::size_t src_size = 0;
    std::size_t target_size = 0;
    std::size_t metadata_size = 0;
    std::string metadata_string;
    std::uint32_t src_crc = 0;
    std::uint32_t target_crc = 0;
    std::uint32_t patch_crc = 0;
    
    enum class Action { SourceRead, TargetRead, SourceCopy, TargetCopy };
    
    struct Command {
        Action action;
        std::uint64_t length;
        std::uint64_t offset_delta;
    };
    
    std::vector<Command> commands;
    std::size_t data_offset = 0;
    
    Result<void> parse() {
        if (patch_data.size() < BPS_HEADER_SIZE + 12) {
            return ErrorInfo{ErrorCode::InvalidPatchFormat, "BPS patch too small"};
        }
        
        if (std::memcmp(patch_data.data(), BPS_MAGIC, BPS_HEADER_SIZE) != 0) {
            return ErrorInfo{ErrorCode::InvalidPatchHeader, "Invalid BPS header"};
        }
        
        std::size_t offset = BPS_HEADER_SIZE;
        
        src_size = decode_bps_num(patch_data, offset);
        target_size = decode_bps_num(patch_data, offset);
        metadata_size = decode_bps_num(patch_data, offset);
        
        if (metadata_size > 0) {
            if (offset + metadata_size > patch_data.size()) {
                return ErrorInfo{ErrorCode::InvalidPatchFormat, "Metadata exceeds patch size"};
            }
            metadata_string.assign(
                reinterpret_cast<const char*>(&patch_data[offset]),
                metadata_size
            );
            offset += metadata_size;
        }
        
        data_offset = offset;
        
        // parse until last 12 bytes
        while (offset + 12 < patch_data.size()) {
            std::uint64_t encoded = decode_bps_num(patch_data, offset);
            
            Command cmd;
            cmd.action = static_cast<Action>(encoded & 3);
            cmd.length = (encoded >> 2) + 1;
            
            if (cmd.action == Action::SourceCopy || cmd.action == Action::TargetCopy) {
                cmd.offset_delta = decode_bps_num(patch_data, offset);
            } else {
                cmd.offset_delta = 0;
            }
            
            commands.push_back(cmd);
        }
        
        if (patch_data.size() < 12) {
            return ErrorInfo{ErrorCode::InvalidPatchFormat, "Missing checksums"};
        }
        
        std::size_t crc_offset = patch_data.size() - 12;
        std::memcpy(&src_crc, &patch_data[crc_offset], 4);
        std::memcpy(&target_crc, &patch_data[crc_offset + 4], 4);
        std::memcpy(&patch_crc, &patch_data[crc_offset + 8], 4);
        
        return Result<void>{};
    }
};

BPSPatch::BPSPatch() : impl_(std::make_unique<Impl>()) {}
BPSPatch::~BPSPatch() = default;

Result<std::unique_ptr<BPSPatch>> BPSPatch::load(const Bytes& patch_data) {
    if (patch_data.size() < BPS_HEADER_SIZE + 12) {
        return ErrorInfo{ErrorCode::InvalidPatchFormat, "BPS patch too small"};
    }
    
    if (std::memcmp(patch_data.data(), BPS_MAGIC, BPS_HEADER_SIZE) != 0) {
        return ErrorInfo{ErrorCode::InvalidPatchHeader, "Invalid BPS header"};
    }
    
    auto patch = std::unique_ptr<BPSPatch>(new BPSPatch());
    patch->impl_->patch_data = patch_data;
    
    auto parse_result = patch->impl_->parse();
    if (!parse_result.is_ok()) {
        return parse_result.error();
    }
    
    return patch;
}

Result<std::unique_ptr<BPSPatch>> BPSPatch::load_from_file(const std::string& path) {
    auto data_result = read_file(path);
    if (!data_result) {
        return data_result.error();
    }
    return load(data_result.value());
}

Result<PatchMetadata> BPSPatch::get_metadata() const {
    PatchMetadata metadata;
    metadata.format = Format::BPS;
    metadata.has_checksums = true;
    metadata.src_size = impl_->src_size;
    metadata.target_size = impl_->target_size;
    metadata.source_checksum = impl_->src_crc;
    metadata.target_checksum = impl_->target_crc;
    
    return metadata;
}

Result<Bytes> BPSPatch::apply(const Bytes& source, const PatchOptions& options) const {

    if (source.size() != impl_->src_size) {
        return ErrorInfo{ErrorCode::SourceSizeMismatch, 
            "Source size mismatch: expected " + std::to_string(impl_->src_size) +
            ", got " + std::to_string(source.size())};
    }
    
    if (options.verify_checksums) {
        std::uint32_t actual_src_crc = calc_crc32(source);
        if (actual_src_crc != impl_->src_crc) {
            return ErrorInfo{ErrorCode::ChecksumMismatch, 
                "Source CRC32 mismatch: expected " + std::to_string(impl_->src_crc) +
                ", got " + std::to_string(actual_src_crc)};
        }
    }
    
    Bytes output;
    output.reserve(impl_->target_size);
    
    std::size_t source_rel_offset = 0;
    std::size_t target_rel_offset = 0;
    std::size_t patch_data_offset = impl_->data_offset;
    
    for (const auto& cmd : impl_->commands) {
        switch (cmd.action) {
            case Impl::Action::SourceRead: {
                if (source_rel_offset + cmd.length > source.size()) {
                    return ErrorInfo{ErrorCode::InvalidPatchFormat, "SourceRead exceeds source size"};
                }
                output.insert(output.end(), 
                    source.begin() + source_rel_offset,
                    source.begin() + source_rel_offset + cmd.length);
                source_rel_offset += cmd.length;
                break;
            }
            
            case Impl::Action::TargetRead: {
                if (patch_data_offset + cmd.length > impl_->patch_data.size() - 12) {
                    return ErrorInfo{ErrorCode::InvalidPatchFormat, "TargetRead exceeds patch data"};
                }
                output.insert(output.end(),
                    impl_->patch_data.begin() + patch_data_offset,
                    impl_->patch_data.begin() + patch_data_offset + cmd.length);
                patch_data_offset += cmd.length;
                break;
            }
            
            case Impl::Action::SourceCopy: {
                std::int64_t offset = static_cast<std::int64_t>(source_rel_offset) + 
                                     ((cmd.offset_delta & 1) ? -static_cast<std::int64_t>(cmd.offset_delta >> 1) 
                                                              : static_cast<std::int64_t>(cmd.offset_delta >> 1));
                if (offset < 0 || static_cast<std::size_t>(offset) + cmd.length > source.size()) {
                    return ErrorInfo{ErrorCode::InvalidPatchFormat, "SourceCopy offset out of bounds"};
                }
                source_rel_offset = offset;
                output.insert(output.end(),
                    source.begin() + offset,
                    source.begin() + offset + cmd.length);
                source_rel_offset += cmd.length;
                break;
            }
            
            case Impl::Action::TargetCopy: {
                std::int64_t offset = static_cast<std::int64_t>(target_rel_offset) + 
                                     ((cmd.offset_delta & 1) ? -static_cast<std::int64_t>(cmd.offset_delta >> 1) 
                                                              : static_cast<std::int64_t>(cmd.offset_delta >> 1));
                if (offset < 0 || static_cast<std::size_t>(offset) >= output.size()) {
                    return ErrorInfo{ErrorCode::InvalidPatchFormat, "TargetCopy offset out of bounds"};
                }

                target_rel_offset = offset;

                for (std::uint64_t i = 0; i < cmd.length; ++i) {
                    if (static_cast<std::size_t>(offset + i) >= output.size()) {
                        return ErrorInfo{ErrorCode::InvalidPatchFormat, "TargetCopy exceeds output size"};
                    }
                    output.push_back(output[offset + i]);
                }
                target_rel_offset += cmd.length;
                break;
            }
        }
    }
    
    if (output.size() != impl_->target_size) {
        return ErrorInfo{ErrorCode::InvalidPatchFormat,
            "Output size mismatch: expected " + std::to_string(impl_->target_size) +
            ", got " + std::to_string(output.size())};
    }
    
    if (options.verify_checksums) {
        std::uint32_t actual_target_crc = calc_crc32(output);
        if (actual_target_crc != impl_->target_crc) {
            return ErrorInfo{ErrorCode::ChecksumMismatch,
                "Target CRC32 mismatch: expected " + std::to_string(impl_->target_crc) +
                ", got " + std::to_string(actual_target_crc)};
        }
    }
    
    return output;
}

Result<void> BPSPatch::apply_to_file( const std::string& source_path, const std::string& output_path, const PatchOptions& options) const {
    
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

Result<void> BPSPatch::validate() const {
    if (impl_->patch_data.size() < BPS_HEADER_SIZE + 12) {
        return ErrorInfo{ErrorCode::InvalidPatchFormat, "BPS patch too small"};
    }
    
    if (std::memcmp(impl_->patch_data.data(), BPS_MAGIC, BPS_HEADER_SIZE) != 0) {
        return ErrorInfo{ErrorCode::InvalidPatchHeader, "Invalid BPS header"};
    }
    
    Bytes data_to_check(impl_->patch_data.begin(), impl_->patch_data.end() - 4);
    std::uint32_t calculated_crc = calc_crc32(data_to_check);
    
    if (calculated_crc != impl_->patch_crc) {
        return ErrorInfo{ErrorCode::ChecksumMismatch,
            "Patch CRC32 mismatch: expected " + std::to_string(impl_->patch_crc) +
            ", got " + std::to_string(calculated_crc)};
    }
    
    return Result<void>{};
}

Result<std::string> BPSPatch::get_metadata_string() const {
    return impl_->metadata_string;
}

Result<void> BPSPatch::verify_checksums(const Bytes& source, const Bytes& target) const {

    std::uint32_t src_crc = calc_crc32(source);
    if (src_crc != impl_->src_crc) {
        return ErrorInfo{ErrorCode::ChecksumMismatch,
            "Source CRC32 mismatch: expected " + std::to_string(impl_->src_crc) +
            ", got " + std::to_string(src_crc)};
    }
    
    std::uint32_t target_crc = calc_crc32(target);
    if (target_crc != impl_->target_crc) {
        return ErrorInfo{ErrorCode::ChecksumMismatch,
            "Target CRC32 mismatch: expected " + std::to_string(impl_->target_crc) +
            ", got " + std::to_string(target_crc)};
    }
    
    return Result<void>{};
}

bool is_bps_format(const Bytes& data) {
    return data.size() >= BPS_HEADER_SIZE && 
           std::memcmp(data.data(), BPS_MAGIC, BPS_HEADER_SIZE) == 0;
}

} // namespace iubpatch
