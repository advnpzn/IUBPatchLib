#pragma once

#include <string>
#include <system_error>
#include <variant>
#include <string_view>
#include <memory>

namespace iubpatch {

enum class ErrorCode {
    Success = 0,
    
    // file I/O
    FileNotFound,
    FileReadError,
    FileWriteError,
    FileAccessDenied,
    FileTooLarge,
    
    // patch format
    InvalidPatchFormat,
    UnsupportedPatchVersion,
    CorruptedPatchData,
    InvalidPatchHeader,
    
    // patching
    InvalidSourceFile,
    SourceSizeMismatch,
    ChecksumMismatch,
    InvalidPatchOffset,
    PatchTooLarge,
    
    // mem
    OutOfMemory,
    MmapFailed,
    
    // misc
    InvalidArgument,
    UnknownError
};

class PatchErrorCategory : public std::error_category {
public:
    const char* name() const noexcept override;
    std::string message(int ev) const override;
};

const std::error_category& patch_category();
std::error_code make_error_code(ErrorCode e);

struct ErrorInfo {
    ErrorCode code;

    std::string message;
    std::string context;
    
    ErrorInfo(ErrorCode c, std::string msg = "", std::string ctx = "")
        : code(c), message(std::move(msg)), context(std::move(ctx)) {}
};

template<typename T>
class [[nodiscard]] Result {
public:
    Result(T value) : data_(std::move(value)) {}
    
    Result(ErrorInfo error) : data_(std::move(error)) {}

    Result(ErrorCode code, std::string message = "") 
        : data_(ErrorInfo{code, std::move(message)}) {}
    
    bool is_ok() const noexcept { 
        return std::holds_alternative<T>(data_); 
    }

    bool is_error() const noexcept { 
        return std::holds_alternative<ErrorInfo>(data_); 
    }

    explicit operator bool() const noexcept {
        return is_ok();
    }
    
    T& value() & { 
        return std::get<T>(data_); 
    }

    const T& value() const & {
        return std::get<T>(data_); 
    }

    T&& value() && {
        return std::get<T>(std::move(data_)); 
    }
    
    const ErrorInfo& error() const & {
        return std::get<ErrorInfo>(data_); 
    }
    ErrorInfo& error() & {
        return std::get<ErrorInfo>(data_);
    }
    
    T value_or(T default_value) const & {
        return is_ok() ? value() : std::move(default_value);
    }
    
private:
    std::variant<T, ErrorInfo> data_;
};


template<>
class [[nodiscard]] Result<void> {
public:
    Result() : error_(nullptr) {}

    Result(ErrorInfo error) : error_(std::make_unique<ErrorInfo>(std::move(error))) {}
    
    Result(ErrorCode code, std::string message = "") 
        : error_(std::make_unique<ErrorInfo>(code, std::move(message))) {}
    
    bool is_ok() const noexcept {
        return error_ == nullptr;
    }

    bool is_error() const noexcept {
        return error_ != nullptr;
    }

    explicit operator bool() const noexcept {
        return is_ok();
    }
    
    const ErrorInfo& error() const {
        return *error_;
    }
    
private:
    std::unique_ptr<ErrorInfo> error_;
};

} // namespace iubpatch


// so that we can use std::error_code with iubpatch::ErrorCode
namespace std {
template<>
struct is_error_code_enum<iubpatch::ErrorCode> : true_type {};
} // namespace std
