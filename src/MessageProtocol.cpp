#include "MessageProtocol.hpp"
#include "ErrorHandler.hpp"

// FlatBuffers includes - these will be available when sparetools-protocols is integrated
// #include <nucleus/protocol/message_envelope_generated.h>
// #include <nucleus/protocol/command_generated.h>
// #include <nucleus/protocol/response_generated.h>
// #include <nucleus/protocol/status_generated.h>
// #include <nucleus/protocol/data_generated.h>
// #include <nucleus/protocol/error_generated.h>

namespace Nucleus {

MessageProtocol::MessageProtocol()
    : next_message_id_(1)
    , initialized_(false)
    , messages_received_(0)
    , messages_sent_(0)
    , errors_count_(0)
    , command_dispatcher_(nullptr)
    , error_handler_(nullptr)
    , status_callback_(nullptr)
    , error_callback_(nullptr) {
}

MessageProtocol::~MessageProtocol() {
    // Clean up any resources if needed
}

bool MessageProtocol::begin() {
    if (initialized_) {
        return true;
    }

    // Initialize serial communication if needed
    // This would be handled by the SerialInterface class

    // Initialize FlatBuffers if needed
    // For now, we'll assume the FlatBuffers library is available

    initialized_ = true;
    return true;
}

void MessageProtocol::setCommandDispatcher(std::unique_ptr<CommandDispatcher> dispatcher) {
    command_dispatcher_ = std::move(dispatcher);
}

void MessageProtocol::setErrorHandler(std::unique_ptr<ErrorHandler> error_handler) {
    error_handler_ = std::move(error_handler);
}

void MessageProtocol::registerCommandCallback(nucleus::protocol::CommandType command_type,
                                             CommandCallback callback) {
    command_callbacks_.emplace_back(command_type, callback);
}

void MessageProtocol::registerStatusCallback(StatusCallback callback) {
    status_callback_ = callback;
}

void MessageProtocol::registerErrorCallback(ErrorCallback callback) {
    error_callback_ = callback;
}

bool MessageProtocol::processMessage(const uint8_t* data, size_t length) {
    if (!initialized_ || !data || length == 0) {
        return false;
    }

    messages_received_++;

    // TODO: Add message framing/deserialization when FlatBuffers headers are available
    // For now, this is a placeholder implementation

    /*
    // Verify data integrity
    if (length < sizeof(uint16_t)) {
        errors_count_++;
        if (error_handler_) {
            error_handler_->reportError(
                nucleus::protocol::ErrorCode::INVALID_DATA,
                "Message too short for checksum",
                nucleus::protocol::ModuleType::SYSTEM,
                false,
                data,
                length
            );
        }
        return false;
    }

    // Extract checksum (assuming it's at the end)
    uint16_t received_checksum = (data[length - 2] << 8) | data[length - 1];
    if (!verifyChecksum(data, length - 2, received_checksum)) {
        errors_count_++;
        if (error_handler_) {
            error_handler_->reportError(
                nucleus::protocol::ErrorCode::INVALID_DATA,
                "Checksum verification failed",
                nucleus::protocol::ModuleType::SYSTEM,
                true, // Checksum errors are recoverable
                data,
                length
            );
        }
        return false;
    }

    // Deserialize FlatBuffers message
    auto envelope = nucleus::protocol::GetMessageEnvelope(data);
    if (!envelope) {
        errors_count_++;
        return false;
    }

    // Validate message
    if (!validateMessage(envelope)) {
        errors_count_++;
        return false;
    }

    // Route message to appropriate handler
    routeMessage(envelope);
    */

    return true;
}

bool MessageProtocol::sendCommand(const nucleus::protocol::Command* command) {
    if (!initialized_ || !command) {
        return false;
    }

    // TODO: Implement when FlatBuffers headers are available
    /*
    // Create message envelope
    auto envelope_builder = std::make_unique<nucleus::protocol::MessageEnvelopeBuilder>();

    // Set envelope fields
    envelope_builder->add_version(1);
    envelope_builder->add_timestamp(millis());
    envelope_builder->add_message_id(getNextMessageId());
    envelope_builder->add_message_type(nucleus::protocol::MessageType_Command);

    // Add command payload
    envelope_builder->add_payload_type(nucleus::protocol::MessagePayload_Command);
    envelope_builder->add_payload(command);

    // Finish and send
    auto envelope = envelope_builder->Finish();
    return sendMessageEnvelope(envelope);
    */

    messages_sent_++;
    return true;
}

bool MessageProtocol::sendResponse(const nucleus::protocol::Response* response, uint32_t message_id) {
    if (!initialized_ || !response) {
        return false;
    }

    // TODO: Implement when FlatBuffers headers are available
    messages_sent_++;
    return true;
}

bool MessageProtocol::sendStatus(const nucleus::protocol::Status* status) {
    if (!initialized_ || !status) {
        return false;
    }

    // TODO: Implement when FlatBuffers headers are available
    messages_sent_++;
    return true;
}

bool MessageProtocol::sendError(const nucleus::protocol::Error* error) {
    if (!initialized_ || !error) {
        return false;
    }

    // TODO: Implement when FlatBuffers headers are available
    messages_sent_++;
    return true;
}

bool MessageProtocol::sendData(const nucleus::protocol::Data* data) {
    if (!initialized_ || !data) {
        return false;
    }

    // TODO: Implement when FlatBuffers headers are available
    messages_sent_++;
    return true;
}

uint32_t MessageProtocol::getNextMessageId() {
    return next_message_id_++;
}

bool MessageProtocol::isReady() const {
    return initialized_;
}

String MessageProtocol::getStats() const {
    String stats = "MessageProtocol Stats:\n";
    stats += "  Initialized: " + String(initialized_ ? "Yes" : "No") + "\n";
    stats += "  Messages Received: " + String(messages_received_) + "\n";
    stats += "  Messages Sent: " + String(messages_sent_) + "\n";
    stats += "  Errors: " + String(errors_count_) + "\n";
    return stats;
}

bool MessageProtocol::validateMessage(const nucleus::protocol::MessageEnvelope* envelope) {
    if (!envelope) {
        if (error_handler_) {
            error_handler_->reportError(
                nucleus::protocol::ErrorCode::INVALID_DATA,
                "Null message envelope",
                nucleus::protocol::ModuleType::SYSTEM,
                false
            );
        }
        return false;
    }

    // TODO: Implement validation logic when FlatBuffers headers are available
    /*
    // Check version compatibility
    if (envelope->version() != 1) {
        if (error_handler_) {
            error_handler_->reportError(
                nucleus::protocol::ErrorCode::INVALID_DATA,
                "Unsupported protocol version: " + String(envelope->version()),
                nucleus::protocol::ModuleType::SYSTEM,
                false
            );
        }
        return false;
    }

    // Validate timestamp (not too old, not in future)
    uint64_t now = millis();
    uint64_t msg_time = envelope->timestamp();
    if (msg_time > now + 10000 || msg_time < now - 300000) { // 10s future, 5min past
        if (error_handler_) {
            error_handler_->reportError(
                nucleus::protocol::ErrorCode::INVALID_DATA,
                "Message timestamp out of valid range",
                nucleus::protocol::ModuleType::SYSTEM,
                true
            );
        }
        return false;
    }

    // Validate message type
    auto msg_type = envelope->message_type();
    if (msg_type < nucleus::protocol::MessageType_Command ||
        msg_type > nucleus::protocol::MessageType_Error) {
        if (error_handler_) {
            error_handler_->reportError(
                nucleus::protocol::ErrorCode::INVALID_DATA,
                "Unknown message type: " + String(static_cast<uint8_t>(msg_type)),
                nucleus::protocol::ModuleType::SYSTEM,
                false
            );
        }
        return false;
    }
    */

    return true;
}

void MessageProtocol::routeMessage(const nucleus::protocol::MessageEnvelope* envelope) {
    if (!envelope) {
        return;
    }

    // TODO: Implement routing when FlatBuffers headers are available
    /*
    switch (envelope->message_type()) {
        case nucleus::protocol::MessageType_Command:
            handleCommand(envelope);
            break;
        case nucleus::protocol::MessageType_Response:
            handleResponse(envelope);
            break;
        case nucleus::protocol::MessageType_Status:
            handleStatus(envelope);
            break;
        case nucleus::protocol::MessageType_Data:
            handleData(envelope);
            break;
        case nucleus::protocol::MessageType_Error:
            handleError(envelope);
            break;
        default:
            // Unknown message type
            errors_count_++;
            break;
    }
    */
}

void MessageProtocol::handleCommand(const nucleus::protocol::MessageEnvelope* envelope) {
    // TODO: Implement when FlatBuffers headers are available
    /*
    auto command = static_cast<const nucleus::protocol::Command*>(
        envelope->payload_as_Command());
    if (!command) {
        return;
    }

    // Use command dispatcher if available
    if (command_dispatcher_) {
        nucleus::protocol::Response* response = command_dispatcher_->dispatchCommand(command);
        if (response) {
            // Send response back
            sendResponse(response, envelope->message_id());
        }
        return;
    }

    // Fallback to legacy callback system
    // Find and call appropriate callback
    for (const auto& callback_pair : command_callbacks_) {
        if (callback_pair.first == command->command_type()) {
            callback_pair.second(command);
            break;
        }
    }
    */
}

void MessageProtocol::handleResponse(const nucleus::protocol::MessageEnvelope* envelope) {
    // TODO: Implement when FlatBuffers headers are available
    // Responses would typically be correlated with pending requests
}

void MessageProtocol::handleStatus(const nucleus::protocol::MessageEnvelope* envelope) {
    if (status_callback_) {
        // TODO: Extract status from envelope
        // status_callback_(status);
    }
}

void MessageProtocol::handleData(const nucleus::protocol::MessageEnvelope* envelope) {
    // TODO: Implement data handling
    // Data messages could be routed to data processing modules
}

void MessageProtocol::handleError(const nucleus::protocol::MessageEnvelope* envelope) {
    if (error_callback_) {
        // TODO: Extract error from envelope
        // error_callback_(error);
    }
}

bool MessageProtocol::sendMessageEnvelope(const nucleus::protocol::MessageEnvelope* envelope) {
    // TODO: Implement when FlatBuffers headers are available
    /*
    // Serialize envelope to buffer
    flatbuffers::FlatBufferBuilder builder;
    builder.Finish(envelope->Pack(builder));

    // Get buffer data
    const uint8_t* buffer = builder.GetBufferPointer();
    size_t size = builder.GetSize();

    // Calculate and append checksum
    uint16_t checksum = calculateChecksum(buffer, size);
    uint8_t checksum_bytes[2] = {static_cast<uint8_t>(checksum >> 8),
                                static_cast<uint8_t>(checksum & 0xFF)};

    // Send data with checksum
    if (!transmitData(buffer, size)) {
        return false;
    }
    if (!transmitData(checksum_bytes, 2)) {
        return false;
    }

    messages_sent_++;
    return true;
    */

    return false;
}

bool MessageProtocol::transmitData(const uint8_t* data, size_t length) {
    // TODO: Implement actual transmission
    // This would interface with the SerialInterface class
    // For now, this is a placeholder

    // Serial.write(data, length);
    // return Serial.availableForWrite() >= 0; // Check if write was successful

    return true;
}

uint16_t MessageProtocol::calculateChecksum(const uint8_t* data, size_t length) {
    uint16_t checksum = 0;
    for (size_t i = 0; i < length; ++i) {
        checksum = (checksum << 8) ^ (checksum + data[i]);
    }
    return checksum;
}

bool MessageProtocol::verifyChecksum(const uint8_t* data, size_t length, uint16_t expected_checksum) {
    uint16_t calculated = calculateChecksum(data, length);
    return calculated == expected_checksum;
}

} // namespace Nucleus