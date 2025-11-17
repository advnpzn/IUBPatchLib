#include "iubpatch/apply.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <patch> <source> <output>\n";
        return 1;
    }
    
    const char* patch_path = argv[1];
    const char* source_path = argv[2];
    const char* output_path = argv[3];
    
    std::cout << "Applying patch...\n";
    std::cout << "  Patch:  " << patch_path << "\n";
    std::cout << "  Source: " << source_path << "\n";
    std::cout << "  Output: " << output_path << "\n";
    
    auto result = iubpatch::apply_patch(patch_path, source_path, output_path);
    
    if (result) {
        std::cout << "Patch applied successfully!\n";
        return 0;
    } else {
        std::cerr << "Error: " << result.error().message << "\n";
        if (!result.error().context.empty()) {
            std::cerr << "Context: " << result.error().context << "\n";
        }
        return 1;
    }
}
