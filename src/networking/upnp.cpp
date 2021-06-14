// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "upnp.h"

#include <miniupnpc.h>
#include <upnpcommands.h>

namespace Networking
{
    UPNP::UPNP(const uint16_t &port, std::string service_name, int timeout, bool v6):
        m_port(port),
        m_service_name(std::move(service_name)),
        m_timeout(timeout),
        m_v6(v6),
        m_active(false),
        m_gateway_urls({}),
        m_upnp_data({0})
    {
        int result;

        UPNPDev *device_list = upnpDiscover(m_timeout, nullptr, nullptr, 0, (m_v6) ? 1 : 0, 2, &result);

        if (device_list == nullptr || result != 0)
        {
            freeUPNPDevlist(device_list);

            return;
        }

        char lan_address[64] = {0};

        result = UPNP_GetValidIGD(device_list, &m_gateway_urls, &m_upnp_data, lan_address, sizeof(lan_address));

        freeUPNPDevlist(device_list);

        if (result != 1)
        {
            return;
        }

        m_lan_address = std::string(lan_address);

        {
            const auto error = add();

            if (!error)
            {
                m_active = true;
            }
        }

        {
            const auto [error, wan_address] = get_external_address();

            if (!error)
            {
                m_wan_address = wan_address;
            }
        }
    }

    UPNP::~UPNP()
    {
        const auto error = del();

        if (!error)
        {
            m_active = false;
        }

        FreeUPNPUrls(&m_gateway_urls);
    }

    bool UPNP::active() const
    {
        return m_active;
    }

    Error UPNP::add()
    {
        const auto port_string = std::to_string(m_port);

        if (UPNP_AddPortMapping(
                m_gateway_urls.controlURL,
                m_upnp_data.first.servicetype,
                port_string.c_str(),
                port_string.c_str(),
                m_lan_address.c_str(),
                m_service_name.c_str(),
                "TCP",
                nullptr,
                "0")
            == 0)
        {
            return MAKE_ERROR(SUCCESS);
        }

        return MAKE_ERROR_MSG(GENERIC_FAILURE, "Could not add UPnP port mapping.");
    }

    Error UPNP::del()
    {
        if (!m_active)
        {
            return MAKE_ERROR_MSG(
                GENERIC_FAILURE, "UPnP is not supported by your network or we were unable to detect its presence.");
        }

        const auto port_string = std::to_string(m_port);

        if (UPNP_DeletePortMapping(
                m_gateway_urls.controlURL, m_upnp_data.first.servicetype, port_string.c_str(), "TCP", nullptr)
            == 0)
        {
            return MAKE_ERROR(SUCCESS);
        }

        return MAKE_ERROR_MSG(GENERIC_FAILURE, "Could not remove UPnP port mapping.");
    }

    std::string UPNP::external_address() const
    {
        return m_wan_address;
    }

    std::tuple<Error, std::string> UPNP::get_external_address()
    {
        if (!m_active)
        {
            return {
                MAKE_ERROR_MSG(
                    GENERIC_FAILURE, "UPnP is not supported by your network or we were unable to detect its presence."),
                {}};
        }

        char wan_address[64] = {0};

        if (UPNP_GetExternalIPAddress(m_gateway_urls.controlURL, m_upnp_data.first.servicetype, wan_address) == 0)
        {
            return {MAKE_ERROR(SUCCESS), std::string(wan_address)};
        }

        return {MAKE_ERROR_MSG(GENERIC_FAILURE, "Could not get external IP address"), {}};
    }

    std::string UPNP::local_address() const
    {
        return m_lan_address;
    }

    uint16_t UPNP::port() const
    {
        return m_port;
    }

    std::string UPNP::service_name() const
    {
        return m_service_name;
    }

    bool UPNP::v6() const
    {
        return m_v6;
    }
} // namespace Networking
