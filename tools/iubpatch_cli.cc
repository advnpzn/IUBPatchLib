#include "iubpatch/apply.h"
#include "iubpatch/patch.h"
#include "iubpatch/api.h"
#include <iostream>
#include <string>
#include <cstring>

void print_usage(const char* program_name) {
    std::cout << "IUBPatchLib v" << iubpatch::get_version_string() << "\n\n";
    std::cout << "Usage:\n";
    std::cout << "  " << program_name << " apply <patch> <source> <output> [options]\n";
    std::cout << "  " << program_name << " info <patch>\n";
    std::cout << "  " << program_name << " validate <patch> <source>\n";
    std::cout << "  " << program_name << " --version\n";
    std::cout << "  " << program_name << " --help\n\n";
    std::cout << "Commands:\n";
    std::cout << "  apply      Apply patch to source file, creating output\n";
    std::cout << "  info       Display patch information\n";
    std::cout << "  validate   Validate patch against source file\n\n";
    std::cout << "Options:\n";
    std::cout << "  --no-checksum  Skip checksum verification\n";
    std::cout << "  --backup       Create backup of original file\n";
    std::cout << "  --no-mmap      Disable memory-mapped I/O\n";
}

void print_version() {
    auto version = iubpatch::get_version();
    std::cout << "IUBPatchLib version " << version.major << "." 
              << version.minor << "." << version.patch << "\n";
    std::cout << "Modern C++ library for IPS, UPS, and BPS patch formats\n";
}

int cmd_apply(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Error: 'apply' requires 3 arguments\n";
        return 1;
    }
    
    const char* patch_path = argv[2];
    const char* source_path = argv[3];
    const char* output_path = argv[4];
    
    iubpatch::PatchOptions options;
    for (int i = 5; i < argc; ++i) {
        if (std::strcmp(argv[i], "--no-checksum") == 0) {
            options.verify_checksums = false;
        } else if (std::strcmp(argv[i], "--backup") == 0) {
            options.create_backup = true;
        } else if (std::strcmp(argv[i], "--no-mmap") == 0) {
            options.use_mmap = false;
        } else {
            std::cerr << "Warning: Unknown option: " << argv[i] << "\n";
        }
    }
    
    std::cout << "Applying patch: " << patch_path << "\n";
    std::cout << "Source: " << source_path << "\n";
    std::cout << "Output: " << output_path << "\n";
    
    auto result = iubpatch::apply_patch(patch_path, source_path, output_path, options);
    
    if (result) {
        std::cout << "Patch applied successfully!\n";
        return 0;
    } else {
        std::cerr << "Error: " << result.error().message << "\n";
        if (!result.error().context.empty()) {
            std::cerr << "  Context: " << result.error().context << "\n";
        }
        return 1;
    }
}

int cmd_info(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Error: 'info' requires a patch file\n";
        return 1;
    }
    
    const char* patch_path = argv[2];
    
    auto info_result = iubpatch::get_patch_info(patch_path);
    
    if (!info_result) {
        std::cerr << "Error: " << info_result.error().message << "\n";
        return 1;
    }
    
    const auto& metadata = info_result.value();
    
    std::cout << "Patch Information:\n";
    std::cout << "  File:   " << patch_path << "\n";
    std::cout << "  Format: " << iubpatch::format_to_string(metadata.format) << "\n";
    
    if (metadata.src_size > 0) {
        std::cout << "  Source Size: " << metadata.src_size << " bytes\n";
    }
    if (metadata.target_size > 0) {
        std::cout << "  Target Size: " << metadata.target_size << " bytes\n";
    }
    
    if (metadata.has_checksums) {
        std::cout << "  Checksums: Available\n";
        std::cout << "    Source: 0x" << std::hex << metadata.source_checksum << std::dec << "\n";
        std::cout << "    Target: 0x" << std::hex << metadata.target_checksum << std::dec << "\n";
    } else {
        std::cout << "  Checksums: Not available\n";
    }
    
    return 0;
}

int cmd_validate(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Error: 'validate' requires patch and source files\n";
        return 1;
    }
    
    const char* patch_path = argv[2];
    const char* source_path = argv[3];
    
    std::cout << "Validating patch: " << patch_path << "\n";
    std::cout << "Against source: " << source_path << "\n";
    
    auto result = iubpatch::validate_patch(patch_path, source_path);
    
    if (result) {
        std::cout << "Patch is valid and can be applied\n";
        return 0;
    } else {
        std::cerr << "Validation failed: " << result.error().message << "\n";
        return 1;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    std::string command = argv[1];
    
    if (command == "--help" || command == "-h") {
        print_usage(argv[0]);
        return 0;
    }
    
    if (command == "--version" || command == "-v") {
        print_version();
        return 0;
    }
    
    if (command == "apply") {
        return cmd_apply(argc, argv);
    }
    
    if (command == "info") {
        return cmd_info(argc, argv);
    }
    
    if (command == "validate") {
        return cmd_validate(argc, argv);
    }
    
    std::cerr << "Error: Unknown command '" << command << "'\n";
    std::cerr << "Run '" << argv[0] << " --help' for usage information\n";
    return 1;
}
