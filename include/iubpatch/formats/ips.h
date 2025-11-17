#pragma once

#include "iubpatch/patch.h"
#include <memory>

namespace iubpatch {

// IPS (International Patching System) format
class IUBPATCH_API IPSPatch : public Patch {
public:
    static Result<std::unique_ptr<IPSPatch>> load(const Bytes& patch_data);

    static Result<std::unique_ptr<IPSPatch>> load_from_file(const std::string& path);
    
    ~IPSPatch() override;
    
    Format get_format() const noexcept override {
        return Format::IPS;
    }

    Result<PatchMetadata> get_metadata() const override;

    Result<Bytes> apply(const Bytes& source, const PatchOptions& options = {}) const override;
    
    Result<void> apply_to_file(
        const std::string& source_path,
        const std::string& output_path,
        const PatchOptions& options = {}
    ) const override;

    Result<void> validate() const override;

    const char* format_name() const noexcept override { return "IPS"; }
    
    // there are cases when an ips patch on a 16 MiB ROM can create 32 MiB output
    // noticed this in some GBA rom hacks, so there's two variants of IPS
    bool is_ips32() const noexcept;
    
private:
    IPSPatch();
    class Impl;
    std::unique_ptr<Impl> impl_;
};

IUBPATCH_API bool is_ips_format(const Bytes& data);

} // namespace iubpatch
