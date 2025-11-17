#include "iubpatch/apply.h"
#include "iubpatch/patch.h"
#include "iubpatch/io.h"
#include <filesystem>
#include <algorithm>

namespace iubpatch {

Result<std::string> create_backup(const std::string& file_path, const PatchOptions& options) {
    if (!options.create_backup) {
        return ErrorInfo{ErrorCode::InvalidArgument, "Backup not requested"};
    }
    
    std::string backup_path = file_path + options.backup_suffix;
    
    try {
        std::filesystem::copy_file(file_path, backup_path, 
            std::filesystem::copy_options::overwrite_existing);
    } catch (const std::filesystem::filesystem_error& e) {
        return ErrorInfo{ErrorCode::FileWriteError, 
            "Failed to create backup: " + std::string(e.what())};
    }
    
    return backup_path;
}

Result<void> verify_output(const std::string& output_path) {
    if (!std::filesystem::exists(output_path)) {
        return ErrorInfo{ErrorCode::FileNotFound, "Output file not found"};
    }
    
    auto size = std::filesystem::file_size(output_path);
    if (size == 0) {
        return ErrorInfo{ErrorCode::InvalidSourceFile, "Output file is empty"};
    }
    
    return Result<void>{};
}

Result<void> apply_patch(
    const std::string& patch_path,
    const std::string& source_path,
    const std::string& output_path,
    const PatchOptions& options
) {

    auto patch_result = load_patch(patch_path);
    if (!patch_result) {
        return patch_result.error();
    }
    
    return patch_result.value()->apply_to_file(source_path, output_path, options);
}

Result<void> apply_patch_inplace(
    const std::string& patch_path,
    const std::string& file_path,
    const PatchOptions& options
) {

    if (options.create_backup) {
        auto backup_result = create_backup(file_path, options);
        if (!backup_result) {
            return backup_result.error();
        }
    }
    
    std::string temp_path = file_path + ".tmp";
    auto result = apply_patch(patch_path, file_path, temp_path, options);
    if (!result) {
        std::filesystem::remove(temp_path);
        return result.error();
    }
    
    try {
        std::filesystem::rename(temp_path, file_path);
    } catch (const std::filesystem::filesystem_error& e) {
        return ErrorInfo{ErrorCode::FileWriteError, 
            "Failed to replace original file: " + std::string(e.what())};
    }
    
    return Result<void>{};
}

Result<void> validate_patch(
    const std::string& patch_path,
    const std::string& source_path,
    const PatchOptions& options
) {

    auto patch_result = load_patch(patch_path);
    if (!patch_result) {
        return patch_result.error();
    }
    

    auto validate_result = patch_result.value()->validate();
    if (!validate_result) {
        return validate_result;
    }
    
    // TODO: verify src file matches expected size/checksum
    // for now tis is not needed, I guess.
    
    return Result<void>{};
}

Result<PatchMetadata> get_patch_info(const std::string& patch_path) {
    auto patch_result = load_patch(patch_path);
    if (!patch_result) {
        return patch_result.error();
    }
    
    return patch_result.value()->get_metadata();
}

} // namespace iubpatch
