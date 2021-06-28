// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "logger.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

logger Logger::create_logger(const std::string &path, const logging_level &level, size_t flush_interval)
{
    // setup logger thread pool
    spdlog::init_thread_pool(8192, 1);

    // set the logs to force a flush ever interval seconds
    spdlog::flush_every(std::chrono::seconds(flush_interval));

    // setup the console output
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    std::vector<spdlog::sink_ptr> sinks {console_sink};

    if (!path.empty())
    {
        // setup the file log output
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path);

        sinks.push_back(file_sink);
    }

    // create the multisink logger
    auto logger = std::make_shared<spdlog::async_logger>(
        "multi_sink", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);

    // set the pattern to what we want it to be
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e UTC] [%^%l%$] %v", spdlog::pattern_time_type::utc);

    // set the default logger level as specified
    logger->set_level(level);

    // register the logger with spdlog
    spdlog::register_logger(logger);

    return logger;
}
