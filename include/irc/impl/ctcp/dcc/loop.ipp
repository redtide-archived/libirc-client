/*
    Name:        irc/impl/ctcp/dcc/loop.ipp
    Purpose:     IRC DCC loop implementation
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/02/03
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_IMPL_CTCP_DCC_LOOP_IPP
#define IRC_IMPL_CTCP_DCC_LOOP_IPP

namespace irc  {
namespace ctcp {
namespace dcc  {

void session::loop( const sys::error_code &ec, size_t bytes )
{
    if( !ec ) BOOST_ASIO_CORO_REENTER( this )
    {
        for( ;; )
        {
            if( !m_connected )
                m_connected = true;

            BOOST_ASIO_CORO_YIELD
            {
                async_write( m_socket, m_buf_write,
                             std::bind( &session::loop, shared_from_this(),
                                        ph::_1, ph::_2 ) );
            }
            BOOST_ASIO_CORO_YIELD
            {
                async_read_until( m_socket, m_buf_read, "\r\n",
                                  std::bind( &session::loop, shared_from_this(),
                                            ph::_1, ph::_2 ) );
            }
            BOOST_ASIO_CORO_YIELD
            {
                m_io_service.post( std::bind( &session::handle_read,
                                            shared_from_this(), ec, bytes ) );
            }
        }
    }
    else if( m_connected )
    {
        disconnect();
    }
}

void session::handle_read( const sys::error_code &ec, size_t bytes )
{
    if( ec )
        return;

    if( m_type == ctcp::dcc::command::chat )
    {
        std::istream in( &m_buf_read );
        std::string  raw_msg;
        std::getline( in, raw_msg );
    }
    else if( m_type == ctcp::dcc::command::send )
    {
        ;
    }

    m_io_service.post( std::bind( &session::loop, shared_from_this(),
                                    ec, bytes ) );
}

} // namespace dcc
} // namespace ctcp
} // namespace irc

#endif // IRC_IMPL_CTCP_DCC_LOOP_IPP
