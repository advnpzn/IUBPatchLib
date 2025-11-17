#include "iubpatch/apply.h"
#include <iostream>
#include <filesystem>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <patch_file> <file1> [file2] ...\n";
        std::cerr << "Applies patch to multiple files in-place\n";
        return 1;
    }
    
    const char* patch_path = argv[1];
    
    iubpatch::PatchOptions options;
    options.create_backup = true;
    
    int success_count = 0;
    int failure_count = 0;
    
    for (int i = 2; i < argc; ++i) {
        std::string file_path = argv[i];
        std::cout << "Patching: " << file_path << "... ";
        
        auto result = iubpatch::apply_patch_inplace(patch_path, file_path, options);
        
        if (result) {
            std::cout << "OK\n";
            ++success_count;
        } else {
            std::cout << "FAILED\n";
            std::cerr << "  Error: " << result.error().message << "\n";
            ++failure_count;
        }
    }
    
    std::cout << "\nResults: " << success_count << " succeeded, " 
              << failure_count << " failed\n";
    
    return failure_count > 0 ? 1 : 0;
}
