#pragma once

#include "iubpatch/api.h"
#include "iubpatch/errors.h"
#include "iubpatch/options.h"
#include "iubpatch/patch.h"
#include <string>

namespace iubpatch {

IUBPATCH_API Result<void> apply_patch(
    const std::string& patch_path,
    const std::string& source_path,
    const std::string& output_path,
    const PatchOptions& options = {}
);

IUBPATCH_API Result<void> apply_patch_inplace(
    const std::string& patch_path,
    const std::string& file_path,
    const PatchOptions& options = {}
);

IUBPATCH_API Result<void> validate_patch(
    const std::string& patch_path,
    const std::string& source_path,
    const PatchOptions& options = {}
);

IUBPATCH_API Result<PatchMetadata> get_patch_info(const std::string& patch_path);

IUBPATCH_API Result<std::string> create_backup(const std::string& file_path, const PatchOptions& options);
IUBPATCH_API Result<void> verify_output(const std::string& output_path);

} // namespace iubpatch
