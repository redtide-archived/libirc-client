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

namespace fsn = boost::fusion;
namespace irc {

void client::loop( const system_error_code &ec, size_t /*bytes*/ )
{
    BOOST_ASIO_CORO_REENTER( this ) { for( ;; ) {
    if( !ec )
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
            m_service.post( std::bind( &client::handle_read, shared_from_this() ) );
        }
    }
    else
    {
        disconnect();
    }}}
}

void client::handle_ctcp( ctcp_message_type ctcp_msg )
{
    ctcp::command ctcp_cmd  = fsn::at_c<0>(ctcp_msg);
    std::string   ctcp_args = fsn::at_c<1>(ctcp_msg);

    if( m_message.command == command::privmsg ) // request
    {
        if( ctcp_cmd == ctcp::command::action )
        {
;//          m_on_action(ctcp_args);
        }
        else if( ctcp_cmd == ctcp::command::dcc )
        {
;//          m_on_dcc_req(ctcp_args);
        }
        else if( ctcp_cmd == ctcp::command::finger )
        {
            ;
        }
        else if( ctcp_cmd == ctcp::command::ping )
        {
            std::string ping_reply = "PING "+ ctcp_args;
            ctcp_reply( m_message.prefix.nickname, ping_reply );
        }
        else if( ctcp_cmd == ctcp::command::time )
        {
            ;
        }
        else if( ctcp_cmd == ctcp::command::version )
        {
;//         m_on_version();
//          ctcp_reply( m_message.prefix.nickname, version() );
        }
    }
    else // command::notice, reply
    {
        ; // TODO: handle CTCP replies
    }
}

void client::handle_message()
{
    if((m_message.command == command::privmsg ||
        m_message.command == command::notice) && m_message.params.size() > 1)
    {
        std::string param = m_message.params.back();
        qi::space_type space;
        ctcp::parser<std::string::const_iterator> ctcp_parser;
        fsn::vector<ctcp::command, std::string>   ctcp_msg;
        std::string::const_iterator first = param.begin();
        std::string::const_iterator last  = param.end();
        qi::phrase_parse( first, last, ctcp_parser, space, ctcp_msg );

        ctcp::command ctcp_cmd = fsn::at_c<0>(ctcp_msg);
        if( ctcp_cmd != ctcp::command::none )
        {
            m_service.dispatch( std::bind( &client::handle_ctcp,
                                            shared_from_this(), ctcp_msg ) );
            return;
        }
        if( m_message.command == command::privmsg )
        {
            if( m_message.params[0].find(m_nickname) != std::string::npos )
                m_on_privmsg( m_message );
            else
                m_on_chanmsg( m_message );
        }
        else
        {
            if( m_message.params[0].find(m_nickname) != std::string::npos )
                m_on_notice( m_message );
            else
                m_on_channtc( m_message );
        }
    }
    else if( m_message.command <= command::err_max )
    {
        m_on_numeric( m_message );
    }
    else if( m_message.command <= command::invite )
    {
        m_on_invite( m_message );
    }
    else if( m_message.command == command::join )
    {
        m_on_join( m_message );
    }
    else if( m_message.command == command::kick )
    {
        m_on_kick( m_message );
    }
    else if( m_message.command == command::nick )
    {
        m_on_nick( m_message );
    }
    else if( m_message.command == command::part )
    {
        m_on_part( m_message );
    }
    else if( m_message.command == command::mode )
    {
        if( is_channel(m_message.params[0]) )
            m_on_chanmode( m_message );
        else
            m_on_usermode( m_message );
    }
    else if( m_message.command == command::ping )
    {
        m_on_ping( m_message );
    }
    else if( m_message.command == command::quit )
    {
        m_on_quit( m_message );
    }
    else if( m_message.command == command::topic )
    {
        m_on_topic( m_message );
    }
    else // Unknown command
    {
        m_on_unknown( m_message );
    }
}

void client::handle_read()
{
    std::istream in( &m_buf_read );
    std::string  message;
    std::getline( in, message );

    if( !message.empty() )
    {
        m_message = irc::message();
        if( parse( message ) )
        {
            m_service.dispatch( std::bind( &client::handle_message,
                                            shared_from_this() ) );
        }
    }

    m_service.post( std::bind( &client::loop, shared_from_this(),
                                system_error_code(), 0 ) );
}

bool client::parse( const std::string &message )
{
    if( message.empty() )
        return false;

    qi::space_type space;
    std::string::const_iterator first = message.begin();
    std::string::const_iterator last  = message.end();
    bool r = qi::phrase_parse( first, last, m_parser, space, m_message );

#ifdef IRC_DEBUG
    std::cout << message << '\n';
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
