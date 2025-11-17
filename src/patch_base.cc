#include "iubpatch/patch.h"
#include "iubpatch/formats/ips.h"
#include "iubpatch/formats/ups.h"
#include "iubpatch/formats/bps.h"
#include <algorithm>

namespace iubpatch {

// magic bytes
static constexpr std::uint8_t IPS_MAGIC[] = {'P', 'A', 'T', 'C', 'H'};
static constexpr std::uint8_t UPS_MAGIC[] = {'U', 'P', 'S', '1'};
static constexpr std::uint8_t BPS_MAGIC[] = {'B', 'P', 'S', '1'};

Result<Format> detect_format_from_memory(const Bytes& patch_data) {
    if (patch_data.size() < 4) {
        return ErrorInfo{ErrorCode::InvalidPatchFormat, "Patch data too small"};
    }
    
    // IPS format
    if (patch_data.size() >= 5 && 
        std::equal(std::begin(IPS_MAGIC), std::end(IPS_MAGIC), patch_data.begin())) {
        return Format::IPS;
    }
    
    // UPS format
    if (patch_data.size() >= 4 && 
        std::equal(std::begin(UPS_MAGIC), std::end(UPS_MAGIC), patch_data.begin())) {
        return Format::UPS;
    }
    
    // BPS format
    if (patch_data.size() >= 4 && 
        std::equal(std::begin(BPS_MAGIC), std::end(BPS_MAGIC), patch_data.begin())) {
        return Format::BPS;
    }
    
    return ErrorInfo{ErrorCode::InvalidPatchFormat, "Unknown patch format"};
}

Result<Format> detect_format(const std::string& patch_path) {
    auto data_result = read_file(patch_path);
    if (!data_result) {
        return data_result.error();
    }
    return detect_format_from_memory(data_result.value());
}

Result<std::unique_ptr<Patch>> load_patch_from_memory(const Bytes& patch_data) {
    auto format_result = detect_format_from_memory(patch_data);
    if (!format_result) {
        return format_result.error();
    }
    
    Format format = format_result.value();
    
    switch (format) {
        case Format::IPS: {
            auto ips_result = IPSPatch::load(patch_data);
            if (!ips_result) {
                return ips_result.error();
            }
            return std::unique_ptr<Patch>(ips_result.value().release());
        }
        case Format::UPS: {
            auto ups_result = UPSPatch::load(patch_data);
            if (!ups_result) {
                return ups_result.error();
            }
            return std::unique_ptr<Patch>(ups_result.value().release());
        }
        case Format::BPS: {
            auto bps_result = BPSPatch::load(patch_data);
            if (!bps_result) {
                return bps_result.error();
            }
            return std::unique_ptr<Patch>(bps_result.value().release());
        }
        default:
            return ErrorInfo{ErrorCode::UnsupportedPatchVersion, "Unsupported patch format"};
    }
}

Result<std::unique_ptr<Patch>> load_patch(const std::string& patch_path) {
    auto data_result = read_file(patch_path);
    if (!data_result) {
        return data_result.error();
    }
    return load_patch_from_memory(data_result.value());
}

const char* format_to_string(Format format) noexcept {
    switch (format) {
        case Format::IPS: return "IPS";
        case Format::UPS: return "UPS";
        case Format::BPS: return "BPS";
        case Format::Unknown:
        default: return "Unknown";
    }
}

} // namespace iubpatch
