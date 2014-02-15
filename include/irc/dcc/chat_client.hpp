/*
    Name:        irc/dcc/chat_client.hpp
    Purpose:     DCC chat client interface
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/02/13
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_DCC_CHAT_CLIENT_HPP
#define IRC_DCC_CHAT_CLIENT_HPP

#include "irc/dcc/client.hpp"

#include <boost/noncopyable.hpp>

#include <memory>

namespace irc  {
namespace dcc  {

class chat_client : public std::enable_shared_from_this< chat_client >,
                    public irc::dcc::client,
                    boost::noncopyable
{
public:
/** Shared DCC chat client pointer */
    typedef std::shared_ptr< chat_client > ptr;

    static ptr make_shared( boost::asio::io_service &ios,
                            irc::prefix             host,
                            const std::string       &address,
                            const std::string       &port );
    ~chat_client() {}
/**
    Sends a DCC chat message, unused with other types.
    @param text The message text.
*/
    void write( const std::string &text );
/**
    Signal fired when recieving a DCC chat message.
    @param func The function to call back.
    @return The connection object to disconnect from the signal.
*/
    template<typename Callback>
    signals::connection on_dcc_message( Callback&& func )
    { return m_on_dcc_msg.connect( std::forward<Callback>(func) ); }

private:
    explicit chat_client(boost::asio::io_service &ios,
                         prefix                  details,
                         const std::string       &addr,
                         const std::string       &port)
    :   client( ios, details, addr, port )
    {
        m_buf_read.prepare(512);
        m_buf_write.prepare(512);
    }

    command do_type() const { return command::chat; }

    void do_read();
    void do_write();

    void handle_read( const boost::system::error_code &ec );
    void handle_write( const boost::system::error_code &ec );

    sig_dcc_msg m_on_dcc_msg;
};

} // namespace dcc
} // namespace irc

#ifdef IRC_CLIENT_HEADER_ONLY
    #include "irc/impl/dcc/chat_client.ipp"
#endif

#endif // IRC_DCC_CHAT_CLIENT_HPP

