#include <gtest/gtest.h>
#include "iubpatch/io.h"
#include <fstream>
#include <filesystem>

using namespace iubpatch;
namespace fs = std::filesystem;

class IOTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = fs::temp_directory_path() / "iubpatch_io_test";
        fs::create_directories(test_dir);
    }

    void TearDown() override {
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
    }

    void create_test_file(const fs::path& path, const std::vector<Byte>& data) {
        std::ofstream ofs(path, std::ios::binary);
        ofs.write(reinterpret_cast<const char*>(data.data()), data.size());
    }

    fs::path test_dir;
};

TEST_F(IOTest, OpenFileReader) {
    auto test_file = test_dir / "test.bin";
    std::vector<Byte> data = {0x01, 0x02, 0x03, 0x04};
    create_test_file(test_file, data);

    auto reader_result = open_file_reader(test_file);
    ASSERT_TRUE(reader_result.is_ok());
    
    auto reader = std::move(reader_result.value());
    EXPECT_EQ(reader->size().value(), 4);
}

TEST_F(IOTest, CreateFileWriter) {
    auto test_file = test_dir / "output.bin";
    
    auto writer_result = create_file_writer(test_file);
    ASSERT_TRUE(writer_result.is_ok());
    
    auto writer = std::move(writer_result.value());
    std::vector<Byte> data = {0xAA, 0xBB, 0xCC};
    
    auto write_result = writer->write(data);
    EXPECT_TRUE(write_result.is_ok());
}
