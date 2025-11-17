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
