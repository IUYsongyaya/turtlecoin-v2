// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_NETWORKING_UPNP_H
#define TURTLECOIN_NETWORKING_UPNP_H

#include <config.h>
#include <errors.h>
#include <miniupnpc.h>

namespace Networking
{
    /**
     * Simple UPNP Helper that auto manages UPnP port forwards for directly
     * connected router(s)/firewall(s) that support it. When the instance
     * is created, the class will attempt to set up the port forward and
     * discover the external IP address for the service. Upon destruction
     * the instance will attempt to delete the port forward before the
     * instance is disposed
     *
     */
    class UPNP
    {
      public:
        /**
         * Creates a new instance of a port mapping to this system if the router/firewall
         * it is connected to supports it
         *
         * @param port
         * @param service_name
         * @param timeout
         * @param v6
         */
        UPNP(
            const uint16_t &port,
            std::string service_name = Configuration::Version::PROJECT_NAME,
            int timeout = 1000,
            bool v6 = false);

        /**
         * Destructor automatically removes the UPnP port mapping
         */
        ~UPNP();

        /**
         * Returns if the UPnP port forward is active
         *
         * @return
         */
        bool active() const;

        /**
         * Returns the external IP address of the service if detected
         *
         * @return
         */
        std::string external_address() const;

        /**
         * Returns the local address of the service if detected
         *
         * @return
         */
        std::string local_address() const;

        /**
         * Returns the port of the service
         *
         * @return
         */
        uint16_t port() const;

        /**
         * Returns the service name
         *
         * @return
         */
        std::string service_name() const;

        /**
         * Whether the service is for IPv6
         *
         * @return
         */
        bool v6() const;

      private:
        Error add();

        Error del();

        std::tuple<Error, std::string> get_external_address();

        std::string m_lan_address, m_service_name, m_wan_address;

        bool m_v6, m_active;

        int m_timeout;

        uint16_t m_port;

        UPNPUrls m_gateway_urls;

        IGDdatas m_upnp_data;
    };

    std::tuple<Error, std::string> get_external_address(bool v6 = false, int timeout = 2000);
} // namespace Networking

#endif
