// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "console.h"

#include "colors.h"

#include <iomanip>
#include <linenoise.hpp>
#include <tools/signal_handler.h>

namespace Utilities
{
    ConsoleHandler::ConsoleHandler(std::string application_name):
        m_name(std::move(application_name)), m_generate_help(true), m_break(false)
    {
        m_command_names.emplace_back("exit");

        m_command_names.emplace_back("help");

        m_command_names.emplace_back("quit");
    }

    void ConsoleHandler::catch_abort()
    {
        ControlSignal::register_handler(
            [&]()
            {
                std::cout << std::endl
                          << COLOR::yellow << "Termination signal caught. Performing hard exit." << COLOR::reset
                          << std::endl
                          << std::endl;

                exit(0);
            });
    }

    void ConsoleHandler::display_help()
    {
        std::vector<std::tuple<std::string, std::string>> options;

        for (const auto &[command, option] : m_commands)
        {
            options.emplace_back(command, option.description);
        }

        options.emplace_back("exit", "Exits the program");

        options.emplace_back("help", "Displays this help message");

        std::cout << std::endl << COLOR::white << m_name << " Help Menu" << COLOR::reset << std::endl;

        Utilities::print_table(options);
    }

    size_t ConsoleHandler::maximum_command_length()
    {
        size_t length = 0;

        for (const auto &command : m_command_names)
        {
            length = std::max(command.length(), length);
        }

        return length;
    }

    void ConsoleHandler::run(const std::string &prompt)
    {
        m_break = false;

        linenoise::SetCompletionCallback(
            [&](const char *data, std::vector<std::string> &completions)
            {
                const auto input = std::string(data);

                for (const auto &command : m_command_names)
                {
                    if (command.compare(0, input.length(), input) == 0)
                    {
                        completions.push_back(command);
                    }
                }
            });

        linenoise::SetHistoryMaxLen(256);

        ControlSignal::register_handler(
            [&]()
            {
                m_break = true;

                std::cout << std::endl
                          << COLOR::yellow << "Termination signal caught. Press ENTER to attempt graceful exit."
                          << COLOR::reset << std::endl
                          << std::endl;
            });

        while (!m_break)
        {
            std::string command;

            auto quit = linenoise::Readline(prompt.c_str(), command);

            if (quit || m_break)
            {
                std::cout << std::endl
                          << COLOR::yellow << "Attempting graceful exit..." << COLOR::reset << std::endl
                          << std::endl;

                break;
            }

            // trim the leading and trailing spaces
            Utilities::str_trim(command, true);

            // split it up into parts (by space)
            auto command_parts = Utilities::str_split(command);

            // the first part is our command
            command = command_parts.front();

            // typing ? is the same as asking for help
            if (command == "?")
            {
                command = "help";
            }

            // erase our command from the front of the vector
            command_parts.erase(command_parts.begin());

            // if the user typed one of these values, we're trying to stop execution
            if (command == "exit" || command == "quit")
            {
                std::cout << std::endl
                          << COLOR::yellow << "Attempting graceful exit..." << COLOR::reset << std::endl
                          << std::endl;

                break;
            }
            // if the user typed help and we're using the auto-generated help, display it
            else if (command == "help" && m_generate_help)
            {
                display_help();

                continue;
            }
            // else, check to see if the command they asked for exists and if so, execute it
            else if (!command.empty() && m_commands.find(command) != m_commands.end())
            {
                linenoise::AddHistory(command.c_str());

                std::visit(
                    [&](auto &&arg)
                    {
                        using T = std::decay_t<decltype(arg)>;

                        if constexpr (std::is_same_v<T, std::function<void(void)>>)
                        {
                            arg();
                        }
                        else if constexpr (std::is_same_v<T, std::function<void(const std::vector<std::string>)>>)
                        {
                            arg(command_parts);
                        }
                    },
                    m_commands.at(command).callback);
            }
        }
    }

    void ConsoleHandler::register_command(
        std::string command,
        const std::string &description,
        const callback_t &callback)
    {
        Utilities::str_trim(command, true);

        if (command.empty())
        {
            return;
        }
        else if (command == "exit" || command == "quit")
        {
            return;
        }
        else if (command == "help")
        {
            m_generate_help = false;
        }

        m_command_names.push_back(command);

        const auto handler = console_command_t(command, description, callback);

        m_commands.insert({command, handler});
    }
} // namespace Utilities
