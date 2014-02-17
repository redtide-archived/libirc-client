/*
    Name:        irc/impl/dcc/chat_client.ipp
    Purpose:     DCC chat client implementation
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/02/13
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_IMPL_DCC_CHAT_CLIENT_IPP
#define IRC_IMPL_DCC_CHAT_CLIENT_IPP

namespace irc  {
namespace dcc  {

chat_client::ptr
chat_client::make_shared(boost::asio::io_service &ios,
                        irc::prefix              details,
                        const std::string        &address,
                        const std::string        &port)
{
    chat_client::ptr new_session( new chat_client(ios, details, address, port) );
    return new_session;
}

void chat_client::write( const std::string &text )
{
    if( m_connected && !text.empty() )
    {
        m_buf_write.consume( m_buf_write.size() );
        std::ostream out( &m_buf_write );
        out << text << "\r\n";

        m_ios.post( std::bind( &chat_client::do_write, shared_from_this() ) );
    }
}

void chat_client::do_write()
{
    boost::asio::async_write
    (
        m_socket,
        m_buf_write,
        std::bind( &chat_client::handle_write, shared_from_this(), 
                    std::placeholders::_1 )
    );
}

void chat_client::do_read()
{
    boost::asio::async_read_until
    (
        m_socket, m_buf_read, '\n',
        std::bind( &chat_client::handle_read, shared_from_this(), 
                    std::placeholders::_1 )
    );
}

void chat_client::handle_read( const boost::system::error_code &ec )
{
    if( !ec )
    {
        if( m_buf_read.size() > 0 )
        {
            std::istream in( &m_buf_read );
            std::string  raw_msg;
            std::getline( in, raw_msg );

            if( raw_msg[raw_msg.size() - 1] == '\r' )
                raw_msg.erase( raw_msg.size() - 1 );

            m_on_dcc_msg( m_prefix, raw_msg );
    //      m_buf_read.consume( m_buf_read.size() );
        }
        m_ios.post( std::bind( &chat_client::do_read, shared_from_this() ) );
    }
    else
    {
        disconnect();
    }
}

void chat_client::handle_write( const boost::system::error_code &ec )
{
    if( ec )
        disconnect();
}

} // namespace dcc
} // namespace irc

#endif // IRC_IMPL_DCC_CHAT_CLIENT_IPP
