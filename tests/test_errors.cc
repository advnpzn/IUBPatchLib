#include <gtest/gtest.h>
#include "iubpatch/errors.h"

using namespace iubpatch;

TEST(ErrorsTest, ResultSuccess) {
    Result<int> result = 42;
    EXPECT_TRUE(result.is_ok());
    EXPECT_EQ(result.value(), 42);
}

TEST(ErrorsTest, ResultError) {
    Result<int> result = ErrorInfo{ErrorCode::InvalidPatchFormat, "test error"};
    EXPECT_FALSE(result.is_ok());
    EXPECT_TRUE(result.is_error());
    EXPECT_EQ(result.error().code, ErrorCode::InvalidPatchFormat);
    EXPECT_EQ(result.error().message, "test error");
}

TEST(ErrorsTest, ResultVoidSuccess) {
    Result<void> result;
    EXPECT_TRUE(result.is_ok());
    EXPECT_FALSE(result.is_error());
}

TEST(ErrorsTest, ResultVoidError) {
    Result<void> result = ErrorInfo{ErrorCode::FileNotFound, "missing file"};
    EXPECT_FALSE(result.is_ok());
    EXPECT_TRUE(result.is_error());
}

TEST(ErrorsTest, ErrorCodeCategories) {
    auto& category = patch_category();
    EXPECT_STREQ(category.name(), "iubpatch");
    
    auto msg1 = category.message(static_cast<int>(ErrorCode::Success));
    EXPECT_FALSE(msg1.empty());
    
    auto msg2 = category.message(static_cast<int>(ErrorCode::InvalidPatchFormat));
    EXPECT_FALSE(msg2.empty());
}
