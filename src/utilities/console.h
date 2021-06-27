// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_UTILITIES_CONSOLE_H
#define TURTLECOIN_UTILITIES_CONSOLE_H

#include "utilities.h"

#include <atomic>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace Utilities
{
    /**
     * Structure that helps us build our interactive console application
     * that holds the command, description, and the callback method
     */
    struct console_command_t
    {
        console_command_t() {}

        console_command_t(
            std::string command,
            std::string description,
            std::function<void(const std::vector<std::string>)> callback):
            command(std::move(command)), description(std::move(description)), callback(std::move(callback))
        {
        }

        std::string command;

        std::string description;

        std::function<void(const std::vector<std::string>)> callback;
    };

    /**
     * A simple interactive console handler that provides a wrapper
     * that makes it easier to work with linenoise (supporting auto-completion)
     * and auto builds a help "menu" if one is not supplied via a registered
     * command
     */
    class ConsoleHandler
    {
      public:
        /**
         * Initiates a new instance of the console helper. If the application
         * name is provided, it will be shown at the top of the help menu;
         * if it is auto generated and if auto-generated,
         * "Application Help Menu" will be displayed at the top of the help
         *
         * @param application_name
         */
        ConsoleHandler(std::string application_name = "Application Help Menu");

        /**
         * Runs the interactive console with the provided prompt
         *
         * @param prompt
         */
        void run(const std::string &prompt = "");

        /**
         * Registers a new command that can be executed from the interactive console
         *
         * @param command
         * @param description
         * @param callback
         */
        void register_command(
            std::string command,
            const std::string &description,
            const std::function<void(const std::vector<std::string>)> &callback);

      private:
        /**
         * Displays the automatically generated "help" menu for a simple menuing
         * system
         */
        void display_help();

        /**
         * Determines the maximum command length for the list of registered
         * commands
         *
         * @return
         */
        size_t maximum_command_length();

        std::map<std::string, console_command_t> m_commands;

        std::vector<std::string> m_command_names;

        std::string m_name;

        std::atomic<bool> m_break, m_generate_help;
    };
} // namespace Utilities

#endif
