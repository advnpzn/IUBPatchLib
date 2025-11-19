#include <gtest/gtest.h>
#include "iubpatch/apply.h"
#include "iubpatch/options.h"
#include <fstream>
#include <filesystem>

using namespace iubpatch;
namespace fs = std::filesystem;

class ApplyTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = fs::temp_directory_path() / "iubpatch_test";
        fs::create_directories(test_dir);
    }

    void TearDown() override {
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
    }

    void create_test_file(const fs::path& path, const std::string& content) {
        std::ofstream ofs(path, std::ios::binary);
        ofs.write(content.data(), content.size());
    }

    fs::path test_dir;
};

TEST_F(ApplyTest, CreateBackup) {
    auto source = test_dir / "source.bin";
    create_test_file(source, "test data");

    PatchOptions opts;
    opts.create_backup = true;
    
    auto backup_path = create_backup(source, opts);
    ASSERT_TRUE(backup_path.is_ok());
    EXPECT_TRUE(fs::exists(backup_path.value()));
}

TEST_F(ApplyTest, VerifyOutput) {
    auto file = test_dir / "test.bin";
    create_test_file(file, "test data");

    Result<void> result = verify_output(file);
    EXPECT_TRUE(result.is_ok());
}

TEST_F(ApplyTest, ValidatePatchChecksumMismatch) {

    auto source_path = test_dir / "source.bin";
    std::string source_data = "wrong";
    create_test_file(source_path, source_data);
    
    auto patch_path = test_dir / "test.ups";
    
    std::vector<uint8_t> patch;
    patch.insert(patch.end(), {'U', 'P', 'S', '1'});
    patch.push_back(0x84);
    patch.push_back(0x84);
    patch.push_back(0x80);
    patch.push_back(0x16);
    patch.push_back(0x00);
    
    // SrcCRC - CRC32("test") = 0xD87F7E0C
    patch.insert(patch.end(), {0x0C, 0x7E, 0x7F, 0xD8});
    // DstCRC - CRC32("best") = 0x66C1988C
    patch.insert(patch.end(), {0x8C, 0x98, 0xC1, 0x66});
    
    auto calc_crc = [](const std::vector<uint8_t>& data) -> uint32_t {
        uint32_t crc = 0xFFFFFFFF;
        for (uint8_t b : data) {
            for (int i = 0; i < 8; i++) {
                uint32_t mask = -(int)(crc & 1);
                crc = (crc >> 1) ^ (0xEDB88320 & mask);
            }
            crc ^= b;
        }
        
        uint32_t c = 0xFFFFFFFF;
        for (uint8_t b : data) {
            c ^= b;
            for (int k = 0; k < 8; k++) {
                c = (c >> 1) ^ (0xEDB88320 & -(c & 1));
            }
        }
        return c ^ 0xFFFFFFFF;
    };
    
    uint32_t patch_crc = calc_crc(patch);
    patch.push_back(patch_crc & 0xFF);
    patch.push_back((patch_crc >> 8) & 0xFF);
    patch.push_back((patch_crc >> 16) & 0xFF);
    patch.push_back((patch_crc >> 24) & 0xFF);
    
    std::ofstream ofs(patch_path, std::ios::binary);
    ofs.write(reinterpret_cast<const char*>(patch.data()), patch.size());
    ofs.close();
    
    PatchOptions opts;
    auto result = validate_patch(patch_path.string(), source_path.string(), opts);
    
    ASSERT_FALSE(result.is_ok());
    
    bool is_expected_error = (result.error().code == ErrorCode::SourceSizeMismatch) || 
                             (result.error().code == ErrorCode::ChecksumMismatch);
    EXPECT_TRUE(is_expected_error) << "Error was: " << result.error().message;

    std::string wrong_content = "tost"; // "test" is correct
    create_test_file(source_path, wrong_content);
    
    result = validate_patch(patch_path.string(), source_path.string(), opts);
    ASSERT_FALSE(result.is_ok());
    EXPECT_EQ(result.error().code, ErrorCode::ChecksumMismatch);
    
    std::string correct_content = "test";
    create_test_file(source_path, correct_content);
    
    result = validate_patch(patch_path.string(), source_path.string(), opts);
    EXPECT_TRUE(result.is_ok()) << "Validation failed for correct file: " << (result.is_ok() ? "" : result.error().message);
}
