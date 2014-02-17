/*
    Name:        irc/dcc/client.hpp
    Purpose:     DCC client interface
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/02/13
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_DCC_CLIENT_HPP
#define IRC_DCC_CLIENT_HPP

#include "irc/dcc/session.hpp"

namespace irc  {
namespace dcc  {

class client : public session
{
public:
    virtual ~client() {}

protected:
    client( boost::asio::io_service &ios,
            irc::prefix             details,
            const std::string       &addr,
            const std::string       &port )
    : session( ios, details, addr, port )
    {
    }

private:
    void do_connect()
    {
        namespace asio = boost::asio;
        namespace ip   = asio::ip;
        namespace ph   = std::placeholders;

        ip::tcp::resolver           resolver( m_ios );
        ip::tcp::resolver::query    query( m_address, m_port );
        ip::tcp::resolver::iterator endpoint_iter = resolver.resolve( query );

        asio::async_connect (
        m_socket, endpoint_iter,
        [this](boost::system::error_code ec, ip::tcp::resolver::iterator)
        {
            if(!ec)
            {
                m_connected = true;
                m_on_connected();
                do_read();
            }
        });
    }
    bool do_active() const { return false; }
};

} // namespace dcc
} // namespace irc

#endif // IRC_DCC_CLIENT_HPP
