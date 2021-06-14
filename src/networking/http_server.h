// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_NETWORKING_HTTP_SERVER_H
#define TURTLECOIN_NETWORKING_HTTP_SERVER_H

#include "http_shared.h"
#include "upnp.h"

#include <errors.h>
#include <httplib.h>

namespace Networking
{
    /**
     * Implements a simple HTTP server that automatically configures a number of
     * security based headers that are useful for RESTful API interfaces. It also
     * sets a default exception handler that consumes the exception and automatically
     * returns a 500 error to the connecting client to prevent possible data leakage
     * from the server instance (ie. throwing error messages back to the client
     * from the stack that *may* potentially contain sensitive information).
     *
     */
    class HTTPServer : public httplib::Server
    {
      public:
        /**
         * Creates a new instance that will serve content within the specified CORS domain
         *
         * @param cors_domain
         */
        HTTPServer(std::string cors_domain = "*");

        /**
         * Destroys the instance
         */
        ~HTTPServer();

        /**
         * Returns the CORS domain that the server is providing in requests
         *
         * @return
         */
        std::string cors_domain() const;

        /**
         * Returns the external IP address for the service (if detected)
         *
         * @return
         */
        std::string external_address() const;

        /**
         * Starts and binds the server to the specified port
         *
         * Note: This method overrides the normal httplib::Server::listen method
         * by launching the server in a separate thread
         *
         * @param host
         * @param port
         * @param socket_flags
         * @return
         */
        bool listen(const std::string &host, int port, int socket_flags = 0);

        /**
         * Parses the request body and returns the json document if it can be parsed
         *
         * @param request
         * @param body_mode
         * @return
         */
        static std::tuple<Error, rapidjson::Document>
            parse_json_body(const httplib::Request &request, const HTTPBodyMode &body_mode = HTTP_BODY_NOT_REQUIRED);

        /**
         * Parses the request body and returns the body if it can be parsed
         *
         * @param request
         * @param body_mode
         * @return
         */
        static std::tuple<Error, std::string>
            parse_text_body(const httplib::Request &request, const HTTPBodyMode &body_mode = HTTP_BODY_NOT_REQUIRED);

        /**
         * Returns the port of this server
         *
         * @return
         */
        uint16_t port() const;

        /**
         * Shuts down the server and stops the thread that it is contained within
         */
        void shutdown();

        /**
         * Returns whether UPnP is active for this instance
         *
         * @return
         */
        bool upnp_active() const;

      private:
        void server_listener();

        std::string m_cors_domain;

        uint16_t m_port;

        std::unique_ptr<UPNP> m_upnp_helper;

        std::thread m_server_thread;
    };
} // namespace Networking

#endif // TURTLECOIN_HTTP_H
