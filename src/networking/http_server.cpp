// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "http_server.h"

namespace Networking
{
    HTTPServer::HTTPServer(logger &logger, std::string cors_domain):
        m_cors_domain(std::move(cors_domain)), m_port(0), m_logger(logger), m_host("")
    {
        // auto set security headers
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
            [&](const auto &request, auto &response, const std::exception &e)
            {
                m_logger->debug("HTTP Internal Server Error: {0}", e.what());

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
        m_logger->debug("Attempting to bind HTTP server to {0}:{1}", host, port);

        // try to bind to the port so that we can get out early if it doesn't work
        if (!bind_to_port(host.c_str(), port, socket_flags))
        {
            m_upnp_helper.reset();

            return false;
        }

        m_port = port;

        m_host = host;

        m_upnp_helper = std::make_unique<UPNP>(m_logger, port, Configuration::Version::PROJECT_NAME + ": HTTP Server");

        // launch the server listener in a thread
        m_server_thread = std::thread(&HTTPServer::server_listener, this);

        m_logger->debug("HTTP server successfully started on {0}:{1}", host, port);

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
            return {MAKE_ERROR(JSON_PARSE_ERROR), std::move(json_body)};
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
        m_logger->debug("Shutting down HTTP server on {0}:{1}...", m_host, m_port);

        if (is_running())
        {
            Server::stop();
        }

        m_logger->trace("HTTPLib instance stopped");

        if (m_server_thread.joinable())
        {
            m_server_thread.join();
        }

        m_logger->debug("HTTP server shutdown complete on {0}:{1}", m_host, m_port);
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
