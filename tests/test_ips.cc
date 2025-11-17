#include <gtest/gtest.h>
#include "iubpatch/formats/ips.h"
#include <vector>

using namespace iubpatch;

TEST(IPSTest, DetectFormat) {
    // IPS magic: "PATCH" + EOF marker
    std::vector<Byte> valid_ips = {'P', 'A', 'T', 'C', 'H', 'E', 'O', 'F'};
    
    auto patch_result = IPSPatch::load(valid_ips);
    EXPECT_TRUE(patch_result.is_ok());
}

TEST(IPSTest, InvalidMagic) {
    std::vector<Byte> invalid = {'B', 'A', 'D', 0x00};
    
    auto patch_result = IPSPatch::load(invalid);
    EXPECT_FALSE(patch_result.is_ok());
}

TEST(IPSTest, ValidateFormat) {
    // Minimal valid IPS: PATCH + EOF
    std::vector<Byte> minimal_ips = {
        'P', 'A', 'T', 'C', 'H',  // Magic
        'E', 'O', 'F'             // EOF marker
    };
    
    auto patch_result = IPSPatch::load(minimal_ips);
    ASSERT_TRUE(patch_result.is_ok());
    
    auto patch = std::move(patch_result.value());
    auto valid = patch->validate();
    EXPECT_TRUE(valid.is_ok());
}
