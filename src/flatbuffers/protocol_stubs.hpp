#pragma once

#include <cstdint>
#include <string>

// Temporary stub definitions for FlatBuffers protocol classes
// These provide minimal interfaces to allow compilation until the actual
// sparetools-protocols package is integrated

namespace nucleus {
namespace protocol {

// Enums
enum class CommandType : uint8_t {
    PING = 0,
    GET_STATUS = 1,
    RESET = 2,
    // Add more as needed
};

enum class ResponseStatus : uint8_t {
    SUCCESS = 0,
    FAILED = 1,
    INVALID_REQUEST = 2,
    // Add more as needed
};

enum class ErrorCode : uint8_t {
    NONE = 0,
    TIMEOUT = 1,
    INVALID_DATA = 2,
    HARDWARE_FAILURE = 3,
    MEMORY_ERROR = 4,
    NETWORK_ERROR = 5,
    PERMISSION_DENIED = 6,
    NOT_SUPPORTED = 7,
    // Add more as needed
};

enum class ModuleType : uint8_t {
    SYSTEM = 0,
    HARDWARE = 1,
    NETWORK = 2,
    // Add more as needed
};

// Stub classes with minimal required methods
class Command {
public:
    Command() = default;
    virtual ~Command() = default;

    virtual CommandType command_type() const { return CommandType::PING; }
};

class Response {
public:
    Response() = default;
    virtual ~Response() = default;

    virtual ResponseStatus status() const { return ResponseStatus::SUCCESS; }
};

class Error {
public:
    Error() = default;
    virtual ~Error() = default;

    virtual ErrorCode error_code() const { return ErrorCode::NONE; }
    virtual bool recoverable() const { return false; }
    virtual const char* message() const { return ""; }
    virtual ModuleType module() const { return ModuleType::SYSTEM; }
};

} // namespace protocol
} // namespace nucleus