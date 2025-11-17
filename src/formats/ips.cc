#include "iubpatch/formats/ips.h"
#include "iubpatch/io.h"
#include <cstring>

namespace iubpatch {

// IPS magic and constants
static constexpr char IPS_MAGIC[] = "PATCH";
static constexpr char IPS_EOF[] = "EOF";
static constexpr std::size_t IPS_HEADER_SIZE = 5;
static constexpr std::size_t IPS_EOF_SIZE = 3;
static constexpr std::size_t IPS_RECORD_HEADER_SIZE = 5;

class IPSPatch::Impl {
public:
    Bytes patch_data;
    bool is_ips32_format = false;
    
    struct Record {
        std::uint32_t offset;
        std::uint16_t size;
        bool is_rle;
        std::uint16_t rle_size;
        Byte rle_value;
        std::vector<Byte> data;
    };

    std::vector<Record> records;
    
    Result<void> parse() {
        records.clear();
        
        if (patch_data.size() < IPS_HEADER_SIZE + IPS_EOF_SIZE) {
            return ErrorInfo{ErrorCode::InvalidPatchFormat, "IPS patch too small"};
        }
        
        std::size_t offset = IPS_HEADER_SIZE;
        
        while (offset + 3 <= patch_data.size()) {

            if (std::memcmp(&patch_data[offset], IPS_EOF, IPS_EOF_SIZE) == 0) {

                if (offset + IPS_EOF_SIZE + 4 <= patch_data.size()) {
                    is_ips32_format = true;
                }
                break;
            }
            
            if (offset + IPS_RECORD_HEADER_SIZE > patch_data.size()) {
                return ErrorInfo{ErrorCode::CorruptedPatchData, "Truncated IPS record header"};
            }
            
            Record rec;

            rec.offset = (static_cast<std::uint32_t>(patch_data[offset]) << 16) |
                        (static_cast<std::uint32_t>(patch_data[offset + 1]) << 8) |
                        static_cast<std::uint32_t>(patch_data[offset + 2]);
            offset += 3;
            
            rec.size = (static_cast<std::uint16_t>(patch_data[offset]) << 8) |
                      static_cast<std::uint16_t>(patch_data[offset + 1]);
            offset += 2;
            
            if (rec.size == 0) {

                if (offset + 3 > patch_data.size()) {
                    return ErrorInfo{ErrorCode::CorruptedPatchData, "Truncated RLE record"};
                }
                rec.is_rle = true;
                rec.rle_size = (static_cast<std::uint16_t>(patch_data[offset]) << 8) |
                              static_cast<std::uint16_t>(patch_data[offset + 1]);
                rec.rle_value = patch_data[offset + 2];
                offset += 3;
            } else {

                if (offset + rec.size > patch_data.size()) {
                    return ErrorInfo{ErrorCode::CorruptedPatchData, "Truncated data record"};
                }
                rec.is_rle = false;
                rec.data.assign(patch_data.begin() + offset, patch_data.begin() + offset + rec.size);
                offset += rec.size;
            }
            
            records.push_back(std::move(rec));
        }
        
        return Result<void>{};
    }
};

IPSPatch::IPSPatch() : impl_(std::make_unique<Impl>()) {}
IPSPatch::~IPSPatch() = default;

Result<std::unique_ptr<IPSPatch>> IPSPatch::load(const Bytes& patch_data) {
    if (patch_data.size() < IPS_HEADER_SIZE + IPS_EOF_SIZE) {
        return ErrorInfo{ErrorCode::InvalidPatchFormat, "IPS patch too small"};
    }
    
    if (std::memcmp(patch_data.data(), IPS_MAGIC, IPS_HEADER_SIZE) != 0) {
        return ErrorInfo{ErrorCode::InvalidPatchHeader, "Invalid IPS header"};
    }
    
    auto patch = std::unique_ptr<IPSPatch>(new IPSPatch());
    patch->impl_->patch_data = patch_data;
    
    auto parse_result = patch->impl_->parse();
    if (!parse_result) {
        return parse_result.error();
    }
    
    return patch;
}

Result<std::unique_ptr<IPSPatch>> IPSPatch::load_from_file(const std::string& path) {
    auto data_result = read_file(path);
    if (!data_result) {
        return data_result.error();
    }
    return load(data_result.value());
}

Result<PatchMetadata> IPSPatch::get_metadata() const {
    PatchMetadata metadata;
    metadata.format = Format::IPS;
    metadata.has_checksums = false;
    
    std::size_t max_offset = 0;
    for (const auto& rec : impl_->records) {
        std::size_t rec_end = rec.offset + (rec.is_rle ? rec.rle_size : rec.size);
        if (rec_end > max_offset) {
            max_offset = rec_end;
        }
    }
    metadata.target_size = max_offset;
    
    return metadata;
}

Result<Bytes> IPSPatch::apply(const Bytes& source, const PatchOptions& options) const {

    Bytes output = source;
    
    for (const auto& rec : impl_->records) {
        std::size_t required_size = rec.offset + (rec.is_rle ? rec.rle_size : rec.size);
        
        if (required_size > output.size()) {
            output.resize(required_size);
        }
        
        if (rec.is_rle) {
            std::fill_n(output.begin() + rec.offset, rec.rle_size, rec.rle_value);
        } else {
            std::copy(rec.data.begin(), rec.data.end(), output.begin() + rec.offset);
        }
    }
    
    return output;
}

Result<void> IPSPatch::apply_to_file(
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

Result<void> IPSPatch::validate() const {

    if (impl_->patch_data.size() < IPS_HEADER_SIZE + IPS_EOF_SIZE) {
        return ErrorInfo{ErrorCode::InvalidPatchFormat, "IPS patch too small"};
    }
    
    if (std::memcmp(impl_->patch_data.data(), IPS_MAGIC, IPS_HEADER_SIZE) != 0) {
        return ErrorInfo{ErrorCode::InvalidPatchHeader, "Invalid IPS header"};
    }
    
    return Result<void>{};
}

bool IPSPatch::is_ips32() const noexcept {
    return impl_->is_ips32_format;
}

bool is_ips_format(const Bytes& data) {
    return data.size() >= IPS_HEADER_SIZE && 
           std::memcmp(data.data(), IPS_MAGIC, IPS_HEADER_SIZE) == 0;
}

} // namespace iubpatch
