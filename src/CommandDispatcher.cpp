#include "CommandDispatcher.hpp"

// FlatBuffers includes - will be available when sparetools-protocols is integrated
// #include <nucleus/protocol/command_generated.h>
// #include <nucleus/protocol/response_generated.h>

namespace Nucleus {

CommandDispatcher::CommandDispatcher()
    : commands_dispatched_(0)
    , commands_handled_(0)
    , commands_failed_(0)
    , unknown_commands_(0) {
}

CommandDispatcher::~CommandDispatcher() {
    // Clean up handlers
    handlers_.clear();
}

bool CommandDispatcher::begin() {
    // Register default system command handlers
    // TODO: Implement when FlatBuffers headers are available

    /*
    // Register ping handler
    registerHandler(nucleus::protocol::CommandType_PING,
                   [this](const nucleus::protocol::Command* cmd) {
                       return handlePing(cmd);
                   });

    // Register status handler
    registerHandler(nucleus::protocol::CommandType_GET_STATUS,
                   [this](const nucleus::protocol::Command* cmd) {
                       return handleGetStatus(cmd);
                   });

    // Register reset handler
    registerHandler(nucleus::protocol::CommandType_RESET,
                   [this](const nucleus::protocol::Command* cmd) {
                       return handleReset(cmd);
                   });
    */

    return true;
}

bool CommandDispatcher::registerHandler(nucleus::protocol::CommandType command_type,
                                       CommandHandler handler) {
    // Check if already registered
    if (findHandler(command_type) != handlers_.end()) {
        return false;
    }

    handlers_.emplace_back(command_type, handler);
    return true;
}

bool CommandDispatcher::unregisterHandler(nucleus::protocol::CommandType command_type) {
    auto it = findHandler(command_type);
    if (it != handlers_.end()) {
        handlers_.erase(it);
        return true;
    }
    return false;
}

nucleus::protocol::Response* CommandDispatcher::dispatchCommand(
    const nucleus::protocol::Command* command) {

    if (!command) {
        commands_failed_++;
        return nullptr; // TODO: Return error response
    }

    commands_dispatched_++;

    // Find handler for this command type
    auto it = findHandler(command->command_type());
    if (it == handlers_.end()) {
        unknown_commands_++;
        return createUnknownCommandResponse(command);
    }

    // Call the handler
    try {
        nucleus::protocol::Response* response = it->second(command);
        if (response) {
            commands_handled_++;
            return response;
        } else {
            commands_failed_++;
            return createHandlerErrorResponse(command, "Handler returned null response");
        }
    } catch (const std::exception& e) {
        commands_failed_++;
        return createHandlerErrorResponse(command, String("Handler exception: ") + e.what());
    } catch (...) {
        commands_failed_++;
        return createHandlerErrorResponse(command, "Handler threw unknown exception");
    }
}

bool CommandDispatcher::isCommandSupported(nucleus::protocol::CommandType command_type) const {
    return std::find_if(handlers_.begin(), handlers_.end(),
                       [command_type](const std::pair<nucleus::protocol::CommandType, CommandDispatcher::CommandHandler>& pair) {
                           return pair.first == command_type;
                       }) != handlers_.end();
}

std::vector<nucleus::protocol::CommandType> CommandDispatcher::getSupportedCommands() const {
    std::vector<nucleus::protocol::CommandType> commands;
    commands.reserve(handlers_.size());

    for (const auto& handler_pair : handlers_) {
        commands.push_back(handler_pair.first);
    }

    return commands;
}

String CommandDispatcher::getStats() const {
    String stats = "CommandDispatcher Stats:\n";
    stats += "  Commands Dispatched: " + String(commands_dispatched_) + "\n";
    stats += "  Commands Handled: " + String(commands_handled_) + "\n";
    stats += "  Commands Failed: " + String(commands_failed_) + "\n";
    stats += "  Unknown Commands: " + String(unknown_commands_) + "\n";
    stats += "  Registered Handlers: " + String(handlers_.size()) + "\n";
    return stats;
}

std::vector<std::pair<nucleus::protocol::CommandType, CommandDispatcher::CommandHandler>>::iterator
CommandDispatcher::findHandler(nucleus::protocol::CommandType command_type) {
    return std::find_if(handlers_.begin(), handlers_.end(),
                       [command_type](const std::pair<nucleus::protocol::CommandType, CommandDispatcher::CommandHandler>& pair) {
                           return pair.first == command_type;
                       });
}

nucleus::protocol::Response* CommandDispatcher::createUnknownCommandResponse(
    const nucleus::protocol::Command* command) {

    // TODO: Implement when FlatBuffers headers are available
    /*
    // Create error response
    auto response_builder = std::make_unique<nucleus::protocol::ResponseBuilder>();
    response_builder->add_status(nucleus::protocol::ResponseStatus_INVALID_REQUEST);

    // Create error info
    auto error_builder = std::make_unique<nucleus::protocol::ErrorInfoBuilder>();
    error_builder->add_error_code(nucleus::protocol::ErrorCode_NOT_SUPPORTED);
    error_builder->add_message("Unknown command type");
    error_builder->add_module(nucleus::protocol::ModuleType_SYSTEM);
    error_builder->add_recoverable(true);

    response_builder->add_error(error_builder->Finish().Union());

    return response_builder->Finish().Union();
    */

    return nullptr;
}

nucleus::protocol::Response* CommandDispatcher::createHandlerErrorResponse(
    const nucleus::protocol::Command* command, const String& error_message) {

    // TODO: Implement when FlatBuffers headers are available
    /*
    // Create error response
    auto response_builder = std::make_unique<nucleus::protocol::ResponseBuilder>();
    response_builder->add_status(nucleus::protocol::ResponseStatus_FAILED);

    // Create error info
    auto error_builder = std::make_unique<nucleus::protocol::ErrorInfoBuilder>();
    error_builder->add_error_code(nucleus::protocol::ErrorCode_HARDWARE_FAILURE);
    error_builder->add_message(error_message.c_str());
    error_builder->add_module(nucleus::protocol::ModuleType_SYSTEM);
    error_builder->add_recoverable(false);

    response_builder->add_error(error_builder->Finish().Union());

    return response_builder->Finish().Union();
    */

    return nullptr;
}

// Default command handlers - will be implemented when FlatBuffers headers are available
/*
nucleus::protocol::Response* CommandDispatcher::handlePing(const nucleus::protocol::Command* command) {
    // TODO: Implement ping handler
    return nullptr;
}

nucleus::protocol::Response* CommandDispatcher::handleGetStatus(const nucleus::protocol::Command* command) {
    // TODO: Implement status handler
    return nullptr;
}

nucleus::protocol::Response* CommandDispatcher::handleReset(const nucleus::protocol::Command* command) {
    // TODO: Implement reset handler
    return nullptr;
}
*/

} // namespace Nucleus