#pragma once

#include <Arduino.h>
#include <memory>
#include <vector>
#include <functional>

// Include protocol stubs for compilation
#include "flatbuffers/protocol_stubs.hpp"

namespace Nucleus {

/**
 * @brief Command Dispatcher for routing commands to appropriate handlers
 *
 * This class manages the registration and dispatching of commands to their
 * respective handler functions. It provides a clean interface for extending
 * command handling without modifying the core protocol logic.
 */
class CommandDispatcher {
public:
    // Command handler function type
    using CommandHandler = std::function<nucleus::protocol::Response*(
        const nucleus::protocol::Command*)>;

    /**
     * @brief Constructor
     */
    CommandDispatcher();

    /**
     * @brief Destructor
     */
    ~CommandDispatcher();

    /**
     * @brief Initialize the command dispatcher
     * @return true on success, false on failure
     */
    bool begin();

    /**
     * @brief Register a command handler
     * @param command_type The type of command to handle
     * @param handler The handler function
     * @return true if registered successfully, false if already registered
     */
    bool registerHandler(nucleus::protocol::CommandType command_type,
                        CommandHandler handler);

    /**
     * @brief Unregister a command handler
     * @param command_type The command type to unregister
     * @return true if unregistered successfully, false if not found
     */
    bool unregisterHandler(nucleus::protocol::CommandType command_type);

    /**
     * @brief Dispatch a command to its handler
     * @param command The command to dispatch
     * @return Response from the handler, or nullptr if no handler found
     */
    nucleus::protocol::Response* dispatchCommand(const nucleus::protocol::Command* command);

    /**
     * @brief Check if a command type is supported
     * @param command_type The command type to check
     * @return true if supported, false otherwise
     */
    bool isCommandSupported(nucleus::protocol::CommandType command_type) const;

    /**
     * @brief Get list of supported command types
     * @return Vector of supported command types
     */
    std::vector<nucleus::protocol::CommandType> getSupportedCommands() const;

    /**
     * @brief Get dispatcher statistics
     * @return String containing statistics
     */
    String getStats() const;

private:
    // Handler storage
    std::vector<std::pair<nucleus::protocol::CommandType, CommandHandler>> handlers_;

    // Statistics
    uint32_t commands_dispatched_;
    uint32_t commands_handled_;
    uint32_t commands_failed_;
    uint32_t unknown_commands_;

    /**
     * @brief Find handler for a command type
     * @param command_type The command type to find
     * @return Iterator to the handler, or end() if not found
     */
    std::vector<std::pair<nucleus::protocol::CommandType, CommandHandler>>::iterator
        findHandler(nucleus::protocol::CommandType command_type);

    /**
     * @brief Create error response for unknown commands
     * @param command The unknown command
     * @return Error response
     */
    nucleus::protocol::Response* createUnknownCommandResponse(
        const nucleus::protocol::Command* command);

    /**
     * @brief Create error response for handler failures
     * @param command The command that failed
     * @param error_message Error description
     * @return Error response
     */
    nucleus::protocol::Response* createHandlerErrorResponse(
        const nucleus::protocol::Command* command,
        const String& error_message);
};

} // namespace Nucleus