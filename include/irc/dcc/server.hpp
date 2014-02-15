/*
    Name:        irc/dcc/server.hpp
    Purpose:     DCC server interface
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/02/13
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_DCC_SERVER_HPP
#define IRC_DCC_SERVER_HPP

#include "irc/dcc/session.hpp"

#include <boost/lexical_cast.hpp>

namespace irc  {
namespace dcc  {

class server : public session
{
protected:
    server( boost::asio::io_service &ios,
            irc::prefix             details,
            const std::string       &addr,
            const std::string       &port )
    :   session( ios, details, addr, port ),
        m_acceptor(ios)
    {
    }

private:
    void do_connect()
    {
        namespace asio = boost::asio;
        namespace ip   = asio::ip;
        namespace ph   = std::placeholders;

        ip::tcp::endpoint ep( ip::tcp::v6(), 0 );
        m_acceptor.open(ep.protocol());
        m_acceptor.set_option(ip::tcp::acceptor::reuse_address(true));
        m_acceptor.bind(ep);
        m_acceptor.listen();

        unsigned short port = m_acceptor.local_endpoint().port();
        m_port = boost::lexical_cast<std::string>(port);

        m_acceptor.async_accept(m_socket, std::bind(&server::loop,
                                        shared_from_this(), ph::_1, 0));
    }
    bool do_active() const { return true; }

    boost::asio::ip::tcp::acceptor m_acceptor;
};

} // namespace dcc
} // 

#endif // IRC_DCC_SERVER_HPP
