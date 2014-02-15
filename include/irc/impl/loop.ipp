/*
    Name:        irc/impl/loop.ipp
    Purpose:     IRC client loop implementation
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/01/16
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_IMPL_LOOP_IPP
#define IRC_IMPL_LOOP_IPP

#include "irc/ctcp/command.hpp"
#include "irc/impl/message_parser.hpp"
#include "irc/impl/ctcp/parser.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/fusion/include/vector.hpp>

#include <functional>

namespace irc {

void client::loop( const boost::system::error_code &ec, size_t bytes )
{
    namespace ph = std::placeholders;

    if( !ec ) BOOST_ASIO_CORO_REENTER( this )
    {
        for( ;; )
        {
            if( !m_connected )
            {
                m_connected = true;
                m_on_connected();

                std::ostream out( &m_buf_write );
                std::string
                nick_cmd = boost::str( boost::format("NICK %1%\r\n") % m_nickname ),
                user_cmd = boost::str( boost::format
                    ("USER %1% unknown unknown :%2%\r\n") % m_username % m_realname );

                out << nick_cmd << user_cmd;
            }
            BOOST_ASIO_CORO_YIELD
            {
                async_write( m_socket, m_buf_write,
                             std::bind( &client::loop, shared_from_this(),
                                        ph::_1, ph::_2 ) );
            }
            BOOST_ASIO_CORO_YIELD
            {
                async_read_until( m_socket, m_buf_read, "\r\n",
                                std::bind( &client::loop, shared_from_this(),
                                            ph::_1, ph::_2 ) );
            }
            BOOST_ASIO_CORO_YIELD
            {
                m_ios.post( std::bind( &client::handle_read,
                                                shared_from_this(), ec, bytes ) );
            }
        }
    }
    else if( m_connected )
    {
        disconnect();
    }
}

void client::handle_message( message &msg, size_t bytes )
{
    namespace dcc = dcc;
    namespace qi  = boost::spirit::qi;
    namespace fsn = boost::fusion;

    if( (msg.params.size() > 1) &&
        (msg.command == command::privmsg ||
         msg.command == command::notice) )
    {
        std::string                               param = msg.params.back();
        std::string::const_iterator               first = param.begin();
        std::string::const_iterator               last  = param.end();
        ctcp::parser<std::string::const_iterator> ctcp_parser;
        qi::space_type                            space;
        fsn::vector<ctcp::command, std::string>   ctcp_msg;

        qi::phrase_parse( first, last, ctcp_parser, space, ctcp_msg );

        ctcp::command ctcp_cmd = fsn::at_c<0>(ctcp_msg);

        if( ctcp_cmd != ctcp::command::none )
        {
            std::string ctcp_args = fsn::at_c<1>(ctcp_msg);
            if( ctcp_cmd == ctcp::command::dcc )
            {
                dcc::request req = dcc_request( ctcp_args );
                if( req.type != dcc::command::none )
                    m_on_dcc_req( msg.prefix, req );
#ifdef IRC_DEBUG
                else
                    std::cerr << "Invalid DCC request: "<< ctcp_args << '\n';
#endif
            }
            else if( msg.command == command::privmsg ) // request
            {
                m_on_ctcp_req( msg, ctcp_cmd, ctcp_args );
            }
            else if( msg.command == command::notice ) // reply
            {
                m_on_ctcp_rep( msg, ctcp_cmd, ctcp_args );
            }
            return;
        }
        if( msg.command == command::privmsg )
        {
            if( msg.params[0] == m_nickname )
                m_on_privmsg( msg );
            else
                m_on_chanmsg( msg );
        }
        else
        {
            if( msg.params[0] == m_nickname )
                m_on_notice( msg );
            else
                m_on_channtc( msg );
        }
    }
    else if( msg.command <= command::err_max )
    {
        m_on_numeric( msg );
    }
    else if( msg.command == command::invite )
    {
        m_on_invite( msg );
    }
    else if( msg.command == command::join )
    {
        m_on_join( msg );
    }
    else if( msg.command == command::kick )
    {
        m_on_kick( msg );
    }
    else if( msg.command == command::nick )
    {
        if( msg.params.size() > 0 && msg.prefix.nickname == m_nickname )
            m_nickname = msg.params[0]; // Update our nickname.

        m_on_nick( msg );
    }
    else if( msg.command == command::part )
    {
        m_on_part( msg );
    }
    else if( msg.command == command::mode )
    {
        if( msg.params.size() > 0 && msg.params[0] == m_nickname )
            m_on_usermode( msg );
        else
            m_on_chanmode( msg );
    }
    else if( msg.command == command::ping )
    {
        m_on_ping( msg );
    }
    else if( msg.command == command::quit )
    {
        m_on_quit( msg );
    }
    else if( msg.command == command::topic )
    {
        m_on_topic( msg );
    }
    else // Unknown command
    {
        m_on_unknown( msg );
    }
}

void client::handle_read( const boost::system::error_code &ec, size_t bytes )
{
    if( !ec )
    {
        std::istream in( &m_buf_read );
        std::string  raw_msg;
        std::getline( in, raw_msg );

        if( !raw_msg.empty() )
        {
            irc::message msg;
            if( parse( raw_msg, msg ) )
            {
                m_ios.post( std::bind( &client::handle_message,
                                        shared_from_this(), msg, bytes ) );
            }
        }
    }
    m_ios.post( std::bind( &client::loop, shared_from_this(), ec, bytes ) );
}

bool client::parse( const std::string &raw_msg, message &msg )
{
    namespace qi = boost::spirit::qi;

    if( raw_msg.empty() )
        return false;

    std::string::const_iterator first = raw_msg.begin();
    std::string::const_iterator last  = raw_msg.end();
    message_parser<std::string::const_iterator> msg_parser;
    qi::space_type space;

    bool r = qi::phrase_parse( first, last, msg_parser, space, msg );

#ifdef IRC_DEBUG
    std::cout << raw_msg << '\n';
#endif
    if( r && first == last )
        return true;

#ifdef IRC_DEBUG
    std::cout << "Parsing failed\n";
#endif
    return false;
}

} // namespace irc

#endif // IRC_IMPL_LOOP_IPP
