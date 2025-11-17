#include "iubpatch/formats/bps.h"
#include "iubpatch/io.h"
#include <cstdint>
#include <cstring>
#include <algorithm>

namespace iubpatch {

// BPS magic
static constexpr char BPS_MAGIC[] = "BPS1";
static constexpr std::size_t BPS_HEADER_SIZE = 4;

// CRC32 table
// https://wiki.osdev.org/CRC32
static const std::uint32_t crc32_table[256] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

static std::uint32_t calc_crc32(const Bytes& data) {
    std::uint32_t crc = 0xFFFFFFFF;
    for (Byte b : data) {
        crc = crc32_table[(crc ^ b) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFF;
}

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
