/*
    Name:        irc/impl/dcc/file_client.ipp
    Purpose:     DCC file client implementation
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/02/16
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_IMPL_DCC_FILE_CLIENT_IPP
#define IRC_IMPL_DCC_FILE_CLIENT_IPP

namespace irc  {
namespace dcc  {

void chat_client::do_write()
{
    ;
}

void chat_client::do_read()
{
    boost::asio::async_read
    (
        m_socket,
        m_buf_read,
//      boost::asio::transfer_all(),
        std::bind( &chat_client::handle_read, shared_from_this(), 
                    std::placeholders::_1 )
    );
}

void chat_client::handle_read( const boost::system::error_code &ec )
{
    if( !ec )
    {
        
    }
    else
    {
        disconnect();
    }
}

} // namespace dcc
} // namespace irc

#endif // IRC_IMPL_DCC_FILE_CLIENT_IPP
