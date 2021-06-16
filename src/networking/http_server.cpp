// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "http_server.h"

namespace Networking
{
    HTTPServer::HTTPServer(std::string cors_domain): m_cors_domain(std::move(cors_domain)), m_port(0)
    {
        // auto set proper security headers
        set_post_routing_handler(
            [this](const auto &req, auto &res)
            {
                res.set_header("Access-Control-Allow-Origin", m_cors_domain);

                res.set_header("X-Requested-With", "*");

                res.set_header(
                    "Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept, User-Agent");

                res.set_header("Access-Control-Allow-Methods", "GET, DELETE, HEAD, POST, PUT, PATCH, OPTIONS");

                res.set_header("Referrer-Policy", "no-referrer");

                res.set_header("Content-Security-Policy", "default-src 'none'");

                res.set_header(
                    "Feature-Policy",
                    "geolocation none;midi none;notifications none;push none;sync-xhr none;microphone none;camera "
                    "none;magnetometer none;gyroscope none;speaker self;vibrate none;fullscreen self;payment none;");

                res.set_header(
                    "Permissions-Policy",
                    "geolocation=(), midi=(), notifications=(), push=(), sync-xhr=(), microphone=(), camera=(), "
                    "magnetometer=(), gyroscope=(), speaker=(self), vibrate=(), fullscreen=(self), payment=()");

                res.set_header("X-Frame-Options", "SAMEORIGIN");

                res.set_header("X-Content-Type-Options", "nosniff");
            });

        set_exception_handler(
            [](const auto &request, auto &response, const std::exception &e)
            {
                response.status = 500;

                response.set_content("500 Internal Server Error", "text/plain");
            });
    }

    HTTPServer::~HTTPServer()
    {
        m_upnp_helper.reset();

        shutdown();
    }

    std::string HTTPServer::cors_domain() const
    {
        return m_cors_domain;
    }

    std::string HTTPServer::external_address() const
    {
        if (!m_upnp_helper)
        {
            return std::string();
        }

        return m_upnp_helper->external_address();
    }

    bool HTTPServer::listen(const std::string &host, int port, int socket_flags)
    {
        // try to bind to the port so that we can get out early if it doesn't work
        if (!bind_to_port(host.c_str(), port, socket_flags))
        {
            m_upnp_helper.reset();

            return false;
        }

        m_port = port;

        m_upnp_helper = std::make_unique<UPNP>(port, Configuration::Version::PROJECT_NAME + ": HTTP Server");

        // launch the server listener in a thread
        m_server_thread = std::thread(&HTTPServer::server_listener, this);

        return true;
    }

    std::tuple<Error, rapidjson::Document>
        HTTPServer::parse_json_body(const httplib::Request &request, const HTTPBodyMode &body_mode)
    {
        rapidjson::Document json_body;

        // if the body is not required, do nothing with it and return
        if (body_mode == HTTP_BODY_NOT_REQUIRED)
        {
            return {MAKE_ERROR(SUCCESS), std::move(json_body)};
        }

        // if the body is empty, and we require one, then return an error
        if (request.body.empty())
        {
            return {MAKE_ERROR(HTTP_BODY_REQUIRED_BUT_NOT_FOUND), std::move(json_body)};
        }

        /**
         * Some methods, such as POST, PATCH, etc may have plain-text style
         * bodies that will not parse as JSON without being enclosed in
         * quotes. Some external libraries properly enclose the values in
         * quotes while others do not. This permits either form to work.
         */
        const auto body_as_json_string = "\"" + request.body + "\"";

        // try to parse the body, if it fails, throw an error
        if (json_body.Parse(request.body.c_str()).HasParseError()
            && json_body.Parse(body_as_json_string.c_str()).HasParseError())
        {
            return {MAKE_ERROR(JSON_DESERIALIZATION_ERROR), std::move(json_body)};
        }

        return {MAKE_ERROR(SUCCESS), std::move(json_body)};
    }

    std::tuple<Error, std::string>
        HTTPServer::parse_text_body(const httplib::Request &request, const HTTPBodyMode &body_mode)
    {
        // if the body is not required, do nothing with it and return
        if (body_mode == HTTP_BODY_NOT_REQUIRED)
        {
            return {MAKE_ERROR(SUCCESS), std::string()};
        }

        // if the body is empty, and we require one, then return an error
        if (request.body.empty())
        {
            return {MAKE_ERROR(HTTP_BODY_REQUIRED_BUT_NOT_FOUND), std::string()};
        }

        return {MAKE_ERROR(SUCCESS), request.body};
    }

    uint16_t HTTPServer::port() const
    {
        return m_port;
    }

    void HTTPServer::server_listener()
    {
        if (!listen_after_bind())
        {
            shutdown();
        }
    }

    void HTTPServer::shutdown()
    {
        if (is_running())
        {
            Server::stop();
        }

        if (m_server_thread.joinable())
        {
            m_server_thread.join();
        }
    }

    bool HTTPServer::upnp_active() const
    {
        if (!m_upnp_helper)
        {
            return false;
        }

        return m_upnp_helper->active();
    }
} // namespace Networking
