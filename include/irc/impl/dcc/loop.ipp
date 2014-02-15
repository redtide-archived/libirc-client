/*
    Name:        irc/impl/dcc/loop.ipp
    Purpose:     IRC DCC loop implementation
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/02/03
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_IMPL_DCC_LOOP_IPP
#define IRC_IMPL_DCC_LOOP_IPP

namespace irc  {
namespace dcc  {

void client::loop( const boost::system::error_code &ec, size_t bytes )
{
    namespace io = boost::asio;
    namespace ph = std::placeholders;

    if( !ec ) BOOST_ASIO_CORO_REENTER( this )
    {
        for( ;; )
        {
            if( !m_connected )
            {
                m_connected = true;
                m_on_connected();
            }
            BOOST_ASIO_CORO_YIELD
            {
                async_write( m_socket, m_buf_write,
                             std::bind( &client::loop, this,
                                        ph::_1, ph::_2 ) );
            }
            BOOST_ASIO_CORO_YIELD
            {
                m_socket.async_read_some( io::buffer(m_data, 1024),
                                        std::bind( &client::loop, this,
                                                    ph::_1, ph::_2 ) );
/*
                async_read_until( m_socket, m_buf_read, '\n',
                                std::bind( &client::loop, this,
                                            ph::_1, ph::_2 ) );
*/
            }
            BOOST_ASIO_CORO_YIELD
            {
                m_ios.post( std::bind( &client::handle_read, this ) );
            }
        }
    }
    else if( m_connected )
    {
        disconnect();
    }
}

} // namespace dcc
} // namespace irc

#endif // IRC_IMPL_DCC_LOOP_IPP
