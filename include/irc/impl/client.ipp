/*
    Name:        irc/impl/client.ipp
    Purpose:     IRC client implementation
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/01/14
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_IMPL_CLIENT_IPP
#define IRC_IMPL_CLIENT_IPP

#include "irc/ctcp/dcc/session.hpp"
#include "irc/impl/ctcp/dcc/parser.hpp"

#include <functional>

namespace ph = std::placeholders;
namespace qi = boost::spirit::qi;
namespace fsn= boost::fusion;
namespace irc {

/*static*/ bool is_channel( const std::string &target )
{
    return( !target.empty() && ((target[0]=='#') || (target[0]=='&') ||
                                (target[0]=='+') || (target[0]=='!')) );
}
/*static*/ std::string version()
{
    return "VERSION irc::client by Andrea Zanellato v0.1";
}

client::ptr client::create( asio::io_service &io_service )
{
    client::ptr new_client( new client( io_service ) );
    return new_client;
}

client::~client()
{
    if( m_connected )
        disconnect();

    m_nickname =
    m_username =
    m_realname = std::string{};

    m_buf_read.consume( m_buf_read.size() );
    m_buf_write.consume( m_buf_write.size() );

    m_lasterror = error_code::success;
}

void client::connect( const std::string &hostname,
                      const std::string &port,
                      const std::string &nickname,
                      const std::string &username,
                      const std::string &realname,
                      const std::string &srv_pwrd )
{
    ip::tcp::resolver           resolver( m_io_service );
    ip::tcp::resolver::query    query( hostname, port );
    ip::tcp::resolver::iterator endpoint_iter = resolver.resolve( query );

    m_nickname = nickname;
    m_username = username;
    m_realname = realname;

    boost::asio::async_connect( m_socket, endpoint_iter,
                                std::bind( &client::loop,
                                            shared_from_this(), ph::_1, 0 ) );
}

void client::disconnect()
{
    if( !m_connected )
    {
        m_lasterror = error_code::invalid_request;
        return;
    }

    m_connected = false;
    m_on_disconnected();

    m_socket.close();
}

void client::send_raw( const std::string &cmd_str )
{
    if( !m_connected )
    {
        m_lasterror = error_code::invalid_request;

        m_io_service.post( std::bind( &client::send_raw,
                                        shared_from_this(), cmd_str ) );
    }
    else
    {
        m_lasterror = error_code::success;

        m_buf_write.consume( m_buf_write.size() );
        std::ostream out( &m_buf_write );
        out << cmd_str << "\r\n";

        m_io_service.post( std::bind( &client::loop, shared_from_this(),
                                        sys::error_code(), 0 ) );
    }
}

void client::action( const std::string &destination, const std::string &message )
{
    if( destination.empty() || message.empty() )
    {
        m_lasterror = error_code::invalid_request;
        m_io_service.post( std::bind( &client::loop, shared_from_this(),
                                        sys::error_code(), 0 ) );
        return;
    }

    send_raw("PRIVMSG "+ destination +" :\x01"+"ACTION "+ message +"\x01");
}

void client::ctcp_request( const std::string &nickname, const std::string &request )
{
    if( nickname.empty() || request.empty() )
    {
        m_lasterror = error_code::invalid_request;
        m_io_service.post( std::bind( &client::loop, shared_from_this(),
                                        sys::error_code(), 0 ) );
        return;
    }
    send_raw("PRIVMSG "+ nickname +" :\x01"+ request +"\x01");
}

void client::ctcp_reply( const std::string &nickname, const std::string &reply )
{
    if( nickname.empty() || reply.empty() )
    {
        m_lasterror = error_code::invalid_request;
        m_io_service.post( std::bind( &client::loop, shared_from_this(),
                                        sys::error_code(), 0 ) );
        return;
    }
    send_raw("NOTICE "+ nickname +" :\x01"+ reply +"\x01");
}

void client::dcc_chat( const std::string &nickname )
{
    ctcp_request( nickname, "DCC CHAT chat 2001:470:c924:00c0:ffee:c0ff:eec0:ffee 12345");
}

void client::dcc_request( const std::string &nickname, const std::string &request )
{
    if( nickname.empty() || request.empty() )
    {
        m_lasterror = error_code::invalid_request;
        m_io_service.post( std::bind( &client::loop, shared_from_this(),
                                    sys::error_code(), 0 ) );
        return;
    }

    using namespace ctcp;
    using dcc_message =
        fsn::vector<dcc::command, std::string, std::string, std::string, long>;

    qi::space_type space;
    dcc_message msg;
    std::string::const_iterator first = request.begin();
    std::string::const_iterator last  = request.end();
    dcc::parser<std::string::const_iterator> dcc_parser;

    if( !qi::phrase_parse( first, last, dcc_parser, space, msg ) )
        return;

    dcc::command type    = fsn::at_c<0>(msg);
    std::string argument = fsn::at_c<1>(msg);
    std::string address  = fsn::at_c<2>(msg);
    std::string port     = fsn::at_c<3>(msg);
    long        size     = fsn::at_c<4>(msg);

    m_on_dcc_req( address, port, type, argument, size );
}

void client::invite( const std::string &nickname, const std::string &channel )
{
    if( nickname.empty() || channel.empty() )
    {
        m_lasterror = error_code::invalid_request;
        m_io_service.post( std::bind( &client::loop, shared_from_this(),
                                        sys::error_code(), 0 ) );
        return;
    }

    send_raw("INVITE "+ nickname +" "+ channel);
}

void client::join( const std::string &channel )
{
    if( channel.empty() )
    {
        m_lasterror = error_code::invalid_request;
        m_io_service.post( std::bind( &client::loop, shared_from_this(),
                                        sys::error_code(), 0 ) );
        return;
    }
    send_raw("JOIN "+ channel);
}

void client::kick( const std::string &nickname, const std::string &channel,
                   const std::string &reason )
{
    if( nickname.empty() || channel.empty() )
    {
        m_lasterror = error_code::invalid_request;
        m_io_service.post( std::bind( &client::loop, shared_from_this(),
                                        sys::error_code(), 0 ) );
        return;
    }

    if( reason.empty() )
        send_raw("KICK "+ channel +" "+ nickname);
    else
        send_raw("KICK "+ channel +" "+ nickname +" :"+ reason);
}

void client::list( const std::string &channels )
{
    send_raw( channels.empty() ? "LIST" : "LIST "+ channels );
}

void client::names( const std::string &channel )
{
    if( channel.empty() )
    {
        m_lasterror = error_code::invalid_request;
        m_io_service.post( std::bind( &client::loop, shared_from_this(),
                                        sys::error_code(), 0 ) );
        return;
    }

    send_raw("NAMES "+ channel);
}

void client::nick( const std::string &newnick )
{
    if( newnick.empty() )
    {
        m_lasterror = error_code::invalid_request;
        m_io_service.post( std::bind( &client::loop, shared_from_this(),
                                        sys::error_code(), 0 ) );
        return;
    }

    send_raw("NICK "+ newnick);
}

void client::notice( const std::string &destination, const std::string &message )
{
    if( destination.empty() || message.empty() )
    {
        m_lasterror = error_code::invalid_request;
        m_io_service.post( std::bind( &client::loop, shared_from_this(),
                                        sys::error_code(), 0 ) );
        return;
    }

    send_raw("NOTICE "+ destination +" :"+ message);
}

void client::part( const std::string &channel )
{
    if( channel.empty() )
    {
        m_lasterror = error_code::invalid_request;
        m_io_service.post( std::bind( &client::loop, shared_from_this(),
                                        sys::error_code(), 0 ) );
        return;
    }

    send_raw("PART "+ channel);
}

void client::pong( const std::string &destination )
{
    send_raw("PONG "+ destination);
}

void client::privmsg( const std::string &destination, const std::string &message )
{
    if( destination.empty() || message.empty() )
    {
        m_lasterror = error_code::invalid_request;
        m_io_service.post( std::bind( &client::loop, shared_from_this(),
                                        sys::error_code(), 0 ) );
        return;
    }

    send_raw("PRIVMSG "+ destination +" :"+ message);
}

void client::quit( const std::string &reason )
{
    std::string cmd_str = reason.empty() ? "QUIT" : "QUIT :"+ reason;
    send_raw( cmd_str );
}

void client::set_mode( const std::string &mode )
{
    if( !mode.empty() )
        send_raw("MODE "+ m_nickname +' '+ mode);
    else
        send_raw("MODE "+ m_nickname);
}

void client::set_channel_mode( const std::string &channel, const std::string &mode )
{
    if( channel.empty() )
    {
        m_lasterror = error_code::invalid_request;
        m_io_service.post( std::bind( &client::loop, shared_from_this(),
                                        sys::error_code(), 0 ) );
        return;
    }

    if( !mode.empty() )
        send_raw("MODE "+ m_nickname +' '+ mode);
    else
        send_raw("MODE "+ m_nickname);
}

void client::topic( const std::string &channel, const std::string &topic )
{
    if( channel.empty() )
    {
        m_lasterror = error_code::invalid_request;
        m_io_service.post( std::bind( &client::loop, shared_from_this(),
                                        sys::error_code(), 0 ) );
        return;
    }

    std::string cmd_str;

    if( topic.empty() )
        cmd_str = "TOPIC "+ channel;
    else
        cmd_str = "TOPIC "+ channel +" :"+ topic;

    send_raw( cmd_str );
}

} // namespace irc

#endif // IRC_IMPL_CLIENT_IPP
