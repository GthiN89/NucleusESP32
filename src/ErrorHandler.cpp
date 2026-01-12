#include "ErrorHandler.hpp"

// FlatBuffers includes - will be available when sparetools-protocols is integrated
// #include <nucleus/protocol/error_generated.h>

namespace Nucleus {

ErrorHandler::ErrorHandler()
    : error_callback_(nullptr)
    , total_errors_(0)
    , recoverable_errors_(0)
    , critical_errors_(0)
    , recovery_attempts_(0)
    , successful_recoveries_(0) {
}

ErrorHandler::~ErrorHandler() {
    // Clean up resources
    recovery_strategies_.clear();
    last_error_.reset();
}

bool ErrorHandler::begin() {
    // Register default recovery strategies for common errors
    // TODO: Implement when FlatBuffers headers are available

    /*
    // Register recovery for timeout errors
    registerRecoveryStrategy(nucleus::protocol::ErrorCode_TIMEOUT,
                           [this](nucleus::protocol::ErrorCode) {
                               // Simple retry logic
                               return true; // Assume retry succeeds
                           });

    // Register recovery for invalid data errors
    registerRecoveryStrategy(nucleus::protocol::ErrorCode_INVALID_DATA,
                           [this](nucleus::protocol::ErrorCode) {
                               // Request retransmission
                               return true;
                           });
    */

    return true;
}

void ErrorHandler::registerErrorCallback(ErrorCallback callback) {
    error_callback_ = callback;
}

void ErrorHandler::registerRecoveryStrategy(nucleus::protocol::ErrorCode error_code,
                                          RecoveryCallback recovery_callback) {
    // Remove existing strategy if present
    auto it = findRecoveryStrategy(error_code);
    if (it != recovery_strategies_.end()) {
        recovery_strategies_.erase(it);
    }

    recovery_strategies_.emplace_back(error_code, recovery_callback);
}

nucleus::protocol::Error* ErrorHandler::reportError(nucleus::protocol::ErrorCode error_code,
                                                  const String& message,
                                                  nucleus::protocol::ModuleType module,
                                                  bool recoverable,
                                                  const uint8_t* context,
                                                  size_t context_size) {

    // Create error object
    nucleus::protocol::Error* error = createError(error_code, message, module,
                                                 recoverable, context, context_size);

    if (error) {
        // Update statistics
        updateStatistics(error);

        // Store as last error
        // TODO: Implement proper error storage when FlatBuffers headers are available
        // last_error_.reset(error);

        // Log the error
        logError(getSeverityFromErrorCode(error_code), error_code, message, module);

        // Call error callback if registered
        if (error_callback_) {
            error_callback_(error);
        }
    }

    return error;
}

bool ErrorHandler::attemptRecovery(const nucleus::protocol::Error* error) {
    if (!error) {
        return false;
    }

    recovery_attempts_++;

    // Find recovery strategy
    auto it = findRecoveryStrategy(error->error_code());
    if (it == recovery_strategies_.end()) {
        // No recovery strategy available
        return false;
    }

    // Attempt recovery
    bool recovered = false;
    try {
        recovered = it->second(error->error_code());
    } catch (...) {
        // Recovery strategy threw an exception
        recovered = false;
    }

    if (recovered) {
        successful_recoveries_++;
    }

    return recovered;
}

void ErrorHandler::logError(Severity severity,
                           nucleus::protocol::ErrorCode error_code,
                           const String& message,
                           nucleus::protocol::ModuleType module) {

    // Format log message
    String log_msg = "[";
    switch (severity) {
        case Severity::DEBUG: log_msg += "DEBUG"; break;
        case Severity::INFO: log_msg += "INFO"; break;
        case Severity::WARNING: log_msg += "WARN"; break;
        case Severity::ERROR: log_msg += "ERROR"; break;
        case Severity::CRITICAL: log_msg += "CRIT"; break;
    }
    log_msg += "] ";

    // Add module name
    log_msg += "Module: ";
    // TODO: Add module name mapping when available
    log_msg += String(static_cast<uint8_t>(module));
    log_msg += " - ";

    // Add error code
    log_msg += "Code: ";
    log_msg += String(static_cast<uint8_t>(error_code));
    log_msg += " - ";

    // Add message
    log_msg += message;

    // Output to appropriate stream based on severity
    switch (severity) {
        case Severity::DEBUG:
        case Severity::INFO:
            // Serial.println(log_msg);
            break;
        case Severity::WARNING:
        case Severity::ERROR:
        case Severity::CRITICAL:
            // Serial.println(log_msg);
            // Could also write to error log file
            break;
    }
}

bool ErrorHandler::hasRecoveryStrategy(nucleus::protocol::ErrorCode error_code) const {
    return findRecoveryStrategy(error_code) != recovery_strategies_.end();
}

String ErrorHandler::getStats() const {
    String stats = "ErrorHandler Stats:\n";
    stats += "  Total Errors: " + String(total_errors_) + "\n";
    stats += "  Recoverable Errors: " + String(recoverable_errors_) + "\n";
    stats += "  Critical Errors: " + String(critical_errors_) + "\n";
    stats += "  Recovery Attempts: " + String(recovery_attempts_) + "\n";
    stats += "  Successful Recoveries: " + String(successful_recoveries_) + "\n";
    stats += "  Recovery Strategies: " + String(recovery_strategies_.size()) + "\n";
    return stats;
}

void ErrorHandler::clearStats() {
    total_errors_ = 0;
    recoverable_errors_ = 0;
    critical_errors_ = 0;
    recovery_attempts_ = 0;
    successful_recoveries_ = 0;
}

const nucleus::protocol::Error* ErrorHandler::getLastError() const {
    return last_error_.get();
}

ErrorHandler::Severity ErrorHandler::getSeverityFromErrorCode(
    nucleus::protocol::ErrorCode error_code) const {

    // TODO: Implement proper severity mapping when FlatBuffers headers are available
    /*
    switch (error_code) {
        case nucleus::protocol::ErrorCode_NONE:
            return Severity::DEBUG;
        case nucleus::protocol::ErrorCode_TIMEOUT:
        case nucleus::protocol::ErrorCode_INVALID_DATA:
            return Severity::WARNING;
        case nucleus::protocol::ErrorCode_HARDWARE_FAILURE:
        case nucleus::protocol::ErrorCode_MEMORY_ERROR:
        case nucleus::protocol::ErrorCode_NETWORK_ERROR:
            return Severity::ERROR;
        case nucleus::protocol::ErrorCode_PERMISSION_DENIED:
        case nucleus::protocol::ErrorCode_NOT_SUPPORTED:
            return Severity::CRITICAL;
        default:
            return Severity::ERROR;
    }
    */

    return Severity::ERROR;
}

nucleus::protocol::Error* ErrorHandler::createError(nucleus::protocol::ErrorCode error_code,
                                                  const String& message,
                                                  nucleus::protocol::ModuleType module,
                                                  bool recoverable,
                                                  const uint8_t* context,
                                                  size_t context_size) {

    // TODO: Implement when FlatBuffers headers are available
    /*
    // Create error using FlatBuffers
    auto error_builder = std::make_unique<nucleus::protocol::ErrorBuilder>();

    error_builder->add_error_code(error_code);
    error_builder->add_message(message.c_str());
    error_builder->add_module(module);
    error_builder->add_recoverable(recoverable);

    if (context && context_size > 0) {
        error_builder->add_context(
            flatbuffers::Vector<uint8_t>(context, context_size));
    }

    return error_builder->Finish().Union();
    */

    return nullptr;
}

void ErrorHandler::updateStatistics(const nucleus::protocol::Error* error) {
    if (!error) {
        return;
    }

    total_errors_++;

    if (error->recoverable()) {
        recoverable_errors_++;
    }

    // Check if this is a critical error
    Severity severity = getSeverityFromErrorCode(error->error_code());
    if (severity == Severity::CRITICAL) {
        critical_errors_++;
    }
}

std::vector<std::pair<nucleus::protocol::ErrorCode, ErrorHandler::RecoveryCallback>>::const_iterator
ErrorHandler::findRecoveryStrategy(nucleus::protocol::ErrorCode error_code) const {
    return std::find_if(recovery_strategies_.begin(), recovery_strategies_.end(),
                       [error_code](const std::pair<nucleus::protocol::ErrorCode, ErrorHandler::RecoveryCallback>& pair) {
                           return pair.first == error_code;
                       });
}

} // namespace Nucleus