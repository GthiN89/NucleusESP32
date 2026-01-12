#pragma once

#include <Arduino.h>
#include <memory>
#include <vector>
#include <functional>

// Include protocol stubs for compilation
#include "flatbuffers/protocol_stubs.hpp"

namespace Nucleus {

/**
 * @brief Error Handler for protocol error management
 *
 * This class manages error conditions, logging, and recovery strategies
 * for the FlatBuffers message protocol.
 */
class ErrorHandler {
public:
    // Error callback types
    using ErrorCallback = std::function<void(const nucleus::protocol::Error*)>;
    using RecoveryCallback = std::function<bool(nucleus::protocol::ErrorCode)>;

    /**
     * @brief Error severity levels
     */
    enum class Severity {
        DEBUG = 0,
        INFO = 1,
        WARNING = 2,
        ERROR = 3,
        CRITICAL = 4
    };

    /**
     * @brief Constructor
     */
    ErrorHandler();

    /**
     * @brief Destructor
     */
    ~ErrorHandler();

    /**
     * @brief Initialize the error handler
     * @return true on success, false on failure
     */
    bool begin();

    /**
     * @brief Register error callback
     * @param callback Function to call when errors occur
     */
    void registerErrorCallback(ErrorCallback callback);

    /**
     * @brief Register recovery strategy for error codes
     * @param error_code The error code to handle
     * @param recovery_callback Function to attempt recovery
     */
    void registerRecoveryStrategy(nucleus::protocol::ErrorCode error_code,
                                 RecoveryCallback recovery_callback);

    /**
     * @brief Report an error
     * @param error_code The error code
     * @param message Human-readable error message
     * @param module The module that generated the error
     * @param recoverable Whether the error is recoverable
     * @param context Additional context data (optional)
     * @return The created error object
     */
    nucleus::protocol::Error* reportError(nucleus::protocol::ErrorCode error_code,
                                         const String& message,
                                         nucleus::protocol::ModuleType module,
                                         bool recoverable = false,
                                         const uint8_t* context = nullptr,
                                         size_t context_size = 0);

    /**
     * @brief Attempt to recover from an error
     * @param error The error to recover from
     * @return true if recovery succeeded, false otherwise
     */
    bool attemptRecovery(const nucleus::protocol::Error* error);

    /**
     * @brief Log an error with severity level
     * @param severity Error severity
     * @param error_code Error code
     * @param message Error message
     * @param module Module that generated the error
     */
    void logError(Severity severity,
                  nucleus::protocol::ErrorCode error_code,
                  const String& message,
                  nucleus::protocol::ModuleType module);

    /**
     * @brief Check if an error code has a recovery strategy
     * @param error_code The error code to check
     * @return true if recovery strategy exists, false otherwise
     */
    bool hasRecoveryStrategy(nucleus::protocol::ErrorCode error_code) const;

    /**
     * @brief Get error statistics
     * @return String containing error statistics
     */
    String getStats() const;

    /**
     * @brief Clear error statistics
     */
    void clearStats();

    /**
     * @brief Get the last error
     * @return Pointer to the last error, or nullptr if no errors
     */
    const nucleus::protocol::Error* getLastError() const;

private:
    // Error callback
    ErrorCallback error_callback_;

    // Recovery strategies
    std::vector<std::pair<nucleus::protocol::ErrorCode, RecoveryCallback>> recovery_strategies_;

    // Statistics
    uint32_t total_errors_;
    uint32_t recoverable_errors_;
    uint32_t critical_errors_;
    uint32_t recovery_attempts_;
    uint32_t successful_recoveries_;

    // Last error storage
    std::unique_ptr<nucleus::protocol::Error> last_error_;

    /**
     * @brief Determine severity from error code
     * @param error_code The error code
     * @return Appropriate severity level
     */
    Severity getSeverityFromErrorCode(nucleus::protocol::ErrorCode error_code) const;

    /**
     * @brief Create error object
     * @param error_code Error code
     * @param message Error message
     * @param module Module
     * @param recoverable Whether recoverable
     * @param context Context data
     * @param context_size Context size
     * @return Created error object
     */
    nucleus::protocol::Error* createError(nucleus::protocol::ErrorCode error_code,
                                         const String& message,
                                         nucleus::protocol::ModuleType module,
                                         bool recoverable,
                                         const uint8_t* context,
                                         size_t context_size);

    /**
     * @brief Update error statistics
     * @param error The error to count
     */
    void updateStatistics(const nucleus::protocol::Error* error);

    /**
     * @brief Find recovery strategy for error code
     * @param error_code The error code
     * @return Iterator to recovery strategy, or end() if not found
     */
    std::vector<std::pair<nucleus::protocol::ErrorCode, RecoveryCallback>>::const_iterator
        findRecoveryStrategy(nucleus::protocol::ErrorCode error_code) const;
};

} // namespace Nucleus