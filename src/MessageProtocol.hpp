#pragma once

#include <Arduino.h>
#include <memory>
#include <vector>
#include <functional>

// Include protocol stubs for compilation
#include "flatbuffers/protocol_stubs.hpp"

// Additional forward declarations for types not in stubs
namespace nucleus {
namespace protocol {

class MessageEnvelope;
class MessageEnvelopeT;
class Status;
class Data;

enum class MessageType : uint8_t;

} // namespace protocol
} // namespace nucleus

// Include component headers for complete types
#include "CommandDispatcher.hpp"
#include "ErrorHandler.hpp"

namespace Nucleus {

/**
 * @brief Message Protocol Handler for FlatBuffers-based communication
 *
 * This class handles serialization/deserialization of messages using FlatBuffers,
 * providing a clean interface for command/response communication between
 * host and device.
 */
class MessageProtocol {
public:
    // Callback types
    using CommandCallback = std::function<void(const nucleus::protocol::Command*)>;
    using StatusCallback = std::function<void(const nucleus::protocol::Status*)>;
    using ErrorCallback = std::function<void(const nucleus::protocol::Error*)>;

    /**
     * @brief Constructor
     */
    MessageProtocol();

    /**
     * @brief Destructor
     */
    ~MessageProtocol();

    /**
     * @brief Set the command dispatcher
     * @param dispatcher Pointer to command dispatcher (takes ownership)
     */
    void setCommandDispatcher(std::unique_ptr<CommandDispatcher> dispatcher);

    /**
     * @brief Set the error handler
     * @param error_handler Pointer to error handler (takes ownership)
     */
    void setErrorHandler(std::unique_ptr<ErrorHandler> error_handler);

    /**
     * @brief Initialize the protocol handler
     * @return true on success, false on failure
     */
    bool begin();

    /**
     * @brief Register a callback for specific command types
     * @param command_type The command type to handle
     * @param callback The callback function
     */
    void registerCommandCallback(nucleus::protocol::CommandType command_type,
                                CommandCallback callback);

    /**
     * @brief Register a status update callback
     * @param callback The callback function
     */
    void registerStatusCallback(StatusCallback callback);

    /**
     * @brief Register an error callback
     * @param callback The callback function
     */
    void registerErrorCallback(ErrorCallback callback);

    /**
     * @brief Process incoming message data
     * @param data Pointer to received data
     * @param length Length of data in bytes
     * @return true if message was processed successfully
     */
    bool processMessage(const uint8_t* data, size_t length);

    /**
     * @brief Send a command message
     * @param command The command to send
     * @return true on success, false on failure
     */
    bool sendCommand(const nucleus::protocol::Command* command);

    /**
     * @brief Send a response message
     * @param response The response to send
     * @param message_id ID of the message being responded to
     * @return true on success, false on failure
     */
    bool sendResponse(const nucleus::protocol::Response* response, uint32_t message_id);

    /**
     * @brief Send a status update
     * @param status The status to send
     * @return true on success, false on failure
     */
    bool sendStatus(const nucleus::protocol::Status* status);

    /**
     * @brief Send error information
     * @param error The error to send
     * @return true on success, false on failure
     */
    bool sendError(const nucleus::protocol::Error* error);

    /**
     * @brief Send raw data
     * @param data The data to send
     * @return true on success, false on failure
     */
    bool sendData(const nucleus::protocol::Data* data);

    /**
     * @brief Get the next message ID for request correlation
     * @return Unique message ID
     */
    uint32_t getNextMessageId();

    /**
     * @brief Check if the protocol is ready for communication
     * @return true if ready, false otherwise
     */
    bool isReady() const;

    /**
     * @brief Get protocol statistics
     * @return String containing statistics
     */
    String getStats() const;

private:
    // Message ID counter for request/response correlation
    uint32_t next_message_id_;

    // Protocol state
    bool initialized_;

    // Statistics
    uint32_t messages_received_;
    uint32_t messages_sent_;
    uint32_t errors_count_;

    // Command dispatcher
    std::unique_ptr<CommandDispatcher> command_dispatcher_;

    // Error handler
    std::unique_ptr<ErrorHandler> error_handler_;

    // Callback storage (legacy - will be phased out)
    std::vector<std::pair<nucleus::protocol::CommandType, CommandCallback>> command_callbacks_;
    StatusCallback status_callback_;
    ErrorCallback error_callback_;

    /**
     * @brief Validate message envelope
     * @param envelope The message envelope to validate
     * @return true if valid, false otherwise
     */
    bool validateMessage(const nucleus::protocol::MessageEnvelope* envelope);

    /**
     * @brief Route message to appropriate handler
     * @param envelope The message envelope
     */
    void routeMessage(const nucleus::protocol::MessageEnvelope* envelope);

    /**
     * @brief Handle command message
     * @param envelope The message envelope containing the command
     */
    void handleCommand(const nucleus::protocol::MessageEnvelope* envelope);

    /**
     * @brief Handle response message
     * @param envelope The message envelope containing the response
     */
    void handleResponse(const nucleus::protocol::MessageEnvelope* envelope);

    /**
     * @brief Handle status message
     * @param envelope The message envelope containing the status
     */
    void handleStatus(const nucleus::protocol::MessageEnvelope* envelope);

    /**
     * @brief Handle data message
     * @param envelope The message envelope containing the data
     */
    void handleData(const nucleus::protocol::MessageEnvelope* envelope);

    /**
     * @brief Handle error message
     * @param envelope The message envelope containing the error
     */
    void handleError(const nucleus::protocol::MessageEnvelope* envelope);

    /**
     * @brief Serialize and send message envelope
     * @param envelope The envelope to send
     * @return true on success, false on failure
     */
    bool sendMessageEnvelope(const nucleus::protocol::MessageEnvelope* envelope);

    /**
     * @brief Low-level data transmission
     * @param data Data to send
     * @param length Length of data
     * @return true on success, false on failure
     */
    bool transmitData(const uint8_t* data, size_t length);

    /**
     * @brief Calculate checksum for data integrity
     * @param data Data to checksum
     * @param length Length of data
     * @return Checksum value
     */
    uint16_t calculateChecksum(const uint8_t* data, size_t length);

    /**
     * @brief Verify data integrity using checksum
     * @param data Data to verify
     * @param length Length of data
     * @param expected_checksum Expected checksum value
     * @return true if checksum matches, false otherwise
     */
    bool verifyChecksum(const uint8_t* data, size_t length, uint16_t expected_checksum);
};

} // namespace Nucleus