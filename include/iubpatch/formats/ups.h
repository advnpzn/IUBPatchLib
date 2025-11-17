#pragma once

#include "iubpatch/patch.h"
#include <memory>

namespace iubpatch {

// UPS (Universal Patching System) format
class IUBPATCH_API UPSPatch : public Patch {
public:
    static Result<std::unique_ptr<UPSPatch>> load(const Bytes& patch_data);
    
    static Result<std::unique_ptr<UPSPatch>> load_from_file(const std::string& path);
    
    ~UPSPatch() override;
    
    Format get_format() const noexcept override {
        return Format::UPS;
    }

    Result<PatchMetadata> get_metadata() const override;

    Result<Bytes> apply(const Bytes& source, const PatchOptions& options = {}) const override;
    
    Result<void> apply_to_file(
        const std::string& source_path,
        const std::string& output_path,
        const PatchOptions& options = {}
    ) const override;

    Result<void> validate() const override;

    const char* format_name() const noexcept override {
        return "UPS";
    }
    
    // we use CRC32 checksums in UPS patches
    Result<void> verify_checksums(const Bytes& source, const Bytes& target) const;
    
private:
    UPSPatch();
    class Impl;
    std::unique_ptr<Impl> impl_;
};

IUBPATCH_API bool is_ups_format(const Bytes& data);

} // namespace iubpatch
