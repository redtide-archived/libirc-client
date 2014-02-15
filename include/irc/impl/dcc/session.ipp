/*
    Name:        irc/impl/dcc/session.ipp
    Purpose:     IRC DCC session implementation
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/02/03
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_IMPL_DCC_SESSION_IPP
#define IRC_IMPL_DCC_SESSION_IPP

#include <boost/lexical_cast.hpp>

namespace irc  {
namespace dcc  {

void session::connect()
{
    namespace asio = boost::asio;
    namespace ip   = asio::ip;
    namespace ph   = std::placeholders;

    if( m_serve )
    {
        ip::tcp::endpoint ep( ip::tcp::v6(), 0 );
        m_acceptor.open(ep.protocol());
        m_acceptor.set_option(ip::tcp::acceptor::reuse_address(true));
        m_acceptor.bind(ep);
        m_acceptor.listen();

        unsigned short port = m_acceptor.local_endpoint().port();
        m_port = boost::lexical_cast<std::string>(port);

        m_acceptor.async_accept(m_socket, std::bind(&session::loop,
                                        shared_from_this(), ph::_1, 0));
    }/*
    else
    {
        ip::tcp::resolver           resolver( m_ios );
        ip::tcp::resolver::query    query( m_address, m_port );
        ip::tcp::resolver::iterator endpoint_iter = resolver.resolve( query );
        asio::async_connect(m_socket, endpoint_iter,
                            std::bind( &session::loop,
                                        shared_from_this(), ph::_1, 0 ) );
    }*/
}

} // namespace dcc
} // namespace irc

#endif // IRC_IMPL_DCC_SESSION_IPP
