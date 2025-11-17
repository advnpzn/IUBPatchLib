#pragma once

#include "iubpatch/patch.h"
#include <memory>

namespace iubpatch {

// BPS (Binary Patching System) format
class IUBPATCH_API BPSPatch : public Patch {
public:
    static Result<std::unique_ptr<BPSPatch>> load(const Bytes& patch_data);

    static Result<std::unique_ptr<BPSPatch>> load_from_file(const std::string& path);
    
    ~BPSPatch() override;
    
    Format get_format() const noexcept override {
        return Format::BPS;
    }

    Result<PatchMetadata> get_metadata() const override;

    Result<Bytes> apply(const Bytes& source, const PatchOptions& options = {}) const override;
    
    Result<void> apply_to_file(
        const std::string& source_path,
        const std::string& output_path,
        const PatchOptions& options = {}
    ) const override;

    Result<void> validate() const override;

    const char* format_name() const noexcept override { return "BPS"; }
    
    Result<std::string> get_metadata_string() const;
    
    Result<void> verify_checksums(const Bytes& source, const Bytes& target) const;
    
private:
    BPSPatch();
    class Impl;
    std::unique_ptr<Impl> impl_;
};

IUBPATCH_API bool is_bps_format(const Bytes& data);

} // namespace iubpatch
