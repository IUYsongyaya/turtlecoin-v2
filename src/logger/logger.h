// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_LOGGER_H
#define TURTLECOIN_LOGGER_H

#include <spdlog/async.h>
#include <spdlog/spdlog.h>

typedef spdlog::level::level_enum logging_level;

typedef std::shared_ptr<spdlog::async_logger> logger;

class Logger
{
  public:
    static logger create_logger(
        const std::string &path = std::string(),
        const logging_level &level = logging_level::info,
        size_t flush_interval = 1);
};

#endif // TURTLECOIN_LOGGER_H
