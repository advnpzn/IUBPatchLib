#include <gtest/gtest.h>
#include "iubpatch/formats/bps.h"
#include <vector>

using namespace iubpatch;

TEST(BPSTest, DetectFormat) {
    // BPS magic: "BPS1" + minimal valid patch
    std::vector<Byte> valid_bps = {
        'B', 'P', 'S', '1',  // Magic
        0x80,                 // src_size = 0 (variable-length)
        0x80,                 // target_size = 0 (variable-length)
        0x80,                 // metadata_size = 0 (variable-length)
        0x00, 0x00, 0x00, 0x00,  // src_crc
        0x00, 0x00, 0x00, 0x00,  // target_crc
        0x00, 0x00, 0x00, 0x00   // patch_crc
    };
    
    auto patch_result = BPSPatch::load(valid_bps);
    EXPECT_TRUE(patch_result.is_ok());
}

TEST(BPSTest, InvalidMagic) {
    std::vector<Byte> invalid = {'B', 'A', 'D', '1'};
    
    auto patch_result = BPSPatch::load(invalid);
    EXPECT_FALSE(patch_result.is_ok());
}
