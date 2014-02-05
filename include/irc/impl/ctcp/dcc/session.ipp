/*
    Name:        irc/impl/ctcp/dcc/session.ipp
    Purpose:     IRC DCC session implementation
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/02/03
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_IMPL_CTCP_DCC_SESSION_IPP
#define IRC_IMPL_CTCP_DCC_SESSION_IPP

namespace ph = std::placeholders;
namespace irc  {
namespace ctcp {
namespace dcc  {

session::ptr session::make_shared( asio::io_service &io_service )
{
    session::ptr new_session( new session( io_service ) );
    return new_session;
}

void session::connect( const std::string &hostname,
                       const std::string &port,
                       ctcp::dcc::command dcctype,
                       const std::string &filename,
                       long               filesize )
{
    ip::tcp::resolver           resolver( m_io_service );
    ip::tcp::resolver::query    query( hostname, port );
    ip::tcp::resolver::iterator endpoint_iter = resolver.resolve( query );

    m_type     = dcctype;
    m_filename = filename;
    m_filesize = filesize;

    asio::async_connect(m_socket, endpoint_iter,
                        std::bind( &session::loop,
                                    shared_from_this(), ph::_1, 0 ) );
}

void session::disconnect()
{
    if( !m_connected )
    {
        m_lasterror = error_code::invalid_request;
        return;
    }

    m_connected = false;
//  m_on_disconnected();

    m_socket.close();
}

session::~session()
{
}

command session::type() const
{
    return m_type;
}

} // namespace dcc
} // namespace ctcp
} // namespace irc

#endif // IRC_IMPL_CTCP_DCC_SESSION_IPP
