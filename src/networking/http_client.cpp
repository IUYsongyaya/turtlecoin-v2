// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "http_client.h"

namespace Networking
{
    std::shared_ptr<httplib::Client> HTTPClient::create_client(
        const std::string &host,
        const uint16_t &port,
        const bool &keepalive,
        const bool &ssl,
        const std::chrono::milliseconds &timeout)
    {
        std::shared_ptr<httplib::Client> client;

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        if (ssl)
        {
            client = std::make_shared<httplib::SSLClient>(host.c_str(), port);
        }
        else
        {
#endif
            client = std::make_shared<httplib::Client>(host.c_str(), port);
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        }
#endif

        client->set_connection_timeout(timeout);

        client->set_keep_alive(keepalive);

        return client;
    }

    std::tuple<Error, rapidjson::Document>
        HTTPClient::parse_json_body(const httplib::Result &result, const HTTPBodyMode &body_mode)
    {
        rapidjson::Document json_body;

        // if the body is not required, do nothing with it and return
        if (body_mode == HTTP_BODY_NOT_REQUIRED)
        {
            return {MAKE_ERROR(SUCCESS), std::move(json_body)};
        }

        // if the body is empty, and we require one, then return an error
        if (result->body.empty())
        {
            return {MAKE_ERROR(HTTP_BODY_REQUIRED_BUT_NOT_FOUND), std::move(json_body)};
        }

        /**
         * Some methods, such as POST, PATCH, etc may have plain-text style
         * bodies that will not parse as JSON without being enclosed in
         * quotes. Some external libraries properly enclose the values in
         * quotes while others do not. This permits either form to work.
         */
        const auto body_as_json_string = "\"" + result->body + "\"";

        // try to parse the body, if it fails, throw an error
        if (json_body.Parse(result->body.c_str()).HasParseError()
            && json_body.Parse(body_as_json_string.c_str()).HasParseError())
        {
            return {MAKE_ERROR(JSON_DESERIALIZATION_ERROR), std::move(json_body)};
        }

        return {MAKE_ERROR(SUCCESS), std::move(json_body)};
    }

    std::tuple<Error, std::string>
        HTTPClient::parse_text_body(const httplib::Result &result, const HTTPBodyMode &body_mode)
    {
        // if the body is not required, do nothing with it and return
        if (body_mode == HTTP_BODY_NOT_REQUIRED)
        {
            return {MAKE_ERROR(SUCCESS), std::string()};
        }

        // if the body is empty, and we require one, then return an error
        if (result->body.empty())
        {
            return {MAKE_ERROR(HTTP_BODY_REQUIRED_BUT_NOT_FOUND), std::string()};
        }

        return {MAKE_ERROR(SUCCESS), result->body};
    }
} // namespace Networking
