#include "iubpatch/apply.h"
#include "iubpatch/patch.h"
#include <iostream>
#include <iomanip>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <patch_file>\n";
        return 1;
    }
    
    const char* patch_path = argv[1];
    auto info_result = iubpatch::get_patch_info(patch_path);
    
    if (!info_result) {
        std::cerr << "Error: " << info_result.error().message << "\n";
        return 1;
    }
    
    const auto& metadata = info_result.value();
    
    std::cout << "Patch Information:\n";
    std::cout << "  Format: " << iubpatch::format_to_string(metadata.format) << "\n";
    std::cout << "  Source Size: " << metadata.src_size << " bytes\n";
    std::cout << "  Target Size: " << metadata.target_size << " bytes\n";
    
    if (metadata.has_checksums) {
        std::cout << "  Source Checksum: 0x" << std::hex << std::setw(8) 
                  << std::setfill('0') << metadata.source_checksum << "\n";
        std::cout << "  Target Checksum: 0x" << std::hex << std::setw(8) 
                  << std::setfill('0') << metadata.target_checksum << "\n";
    } else {
        std::cout << "  Checksums: Not available\n";
    }
    
    return 0;
}
