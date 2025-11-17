#include <gtest/gtest.h>
#include "iubpatch/formats/ups.h"
#include <vector>

using namespace iubpatch;

TEST(UPSTest, DetectFormat) {
    // UPS magic: "UPS1" + minimal valid patch
    std::vector<Byte> valid_ups = {
        'U', 'P', 'S', '1',  // Magic
        0x80,                 // src_size = 0 (variable-length)
        0x80,                 // target_size = 0 (variable-length)
        0x00, 0x00, 0x00, 0x00,  // src_crc
        0x00, 0x00, 0x00, 0x00,  // target_crc
        0x00, 0x00, 0x00, 0x00   // patch_crc
    };
    
    auto patch_result = UPSPatch::load(valid_ups);
    EXPECT_TRUE(patch_result.is_ok());
}

TEST(UPSTest, InvalidMagic) {
    std::vector<Byte> invalid = {'B', 'A', 'D', '1'};
    
    auto patch_result = UPSPatch::load(invalid);
    EXPECT_FALSE(patch_result.is_ok());
}
