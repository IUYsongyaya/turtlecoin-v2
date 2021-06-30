// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_NETWORKING_HTTP_CLIENT_H
#define TURTLECOIN_NETWORKING_HTTP_CLIENT_H

#include "http_shared.h"

#include <config.h>
#include <errors.h>
#include <httplib.h>

namespace Networking
{
    /**
     * Implements a simple HTTP/s client that handles a few default options via
     * the construction and provides a common interface for creating both a
     * HTTP and HTTPS (TLS) client in the same method calls. Also provides
     * a number of helper methods for parsing result bodies.
     *
     */
    class HTTPClient
    {
      public:
        /**
         * Creates a new HTTP client, sets the default connect timeout to the timeout specified
         * and enables keepalives if requested.
         *
         * Note: If SSL is specified AND the code is target is built with OpenSSL support, it
         * will create a SSL capable client.
         *
         * @param host
         * @param port
         * @param ssl
         * @param timeout in milliseconds
         * @return
         */
        static std::shared_ptr<httplib::Client> create_client(
            const std::string &host,
            const uint16_t &port,
            const bool &keepalive = true,
            const bool &ssl = false,
            int timeout = Configuration::DEFAULT_CONNECTION_TIMEOUT);

        /**
         * Parses the result body and returns the json document if it can be parsed
         *
         * @param result
         * @param body_mode
         * @return
         */
        static std::tuple<Error, rapidjson::Document>
            parse_json_body(const httplib::Result &result, const HTTPBodyMode &body_mode = HTTP_BODY_NOT_REQUIRED);

        /**
         * Parses the result body and returns the body if it can be parsed
         *
         * @param result
         * @param body_mode
         * @return
         */
        static std::tuple<Error, std::string>
            parse_text_body(const httplib::Result &result, const HTTPBodyMode &body_mode = HTTP_BODY_NOT_REQUIRED);
    };
} // namespace Networking

#endif
