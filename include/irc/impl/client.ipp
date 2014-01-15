/*
    Name:        irc/impl/client.ipp
    Purpose:     IRC client implementation
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/01/14
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_IMPL_CLIENT_HPP
#define IRC_IMPL_CLIENT_HPP

namespace irc {

client::ptr client::create( io_service &io_service )
{
    client::ptr new_client( new client( io_service ) );
    return new_client;
}

client::~client()
{
    if( m_connected )
        disconnect();
}

void client::connect( const std::string &hostname,
                      const std::string &port,
                      const std::string &nickname,
                      const std::string &username,
                      const std::string &realname,
                      const std::string &srv_pwrd )
{
    resolver           resolver( m_service );
    resolver::query    query( hostname, port );
    resolver::iterator endpoint_iter = resolver.resolve( query );

    m_nickname = nickname;
    m_username = username;
    m_realname = realname;

    boost::asio::async_connect( m_socket, endpoint_iter,
                                std::bind( &client::handle_connect,
                                            shared_from_this(), ph::_1 ) );
}

void client::disconnect()
{
    if( !m_connected )
    {
        m_lasterror = error_code::invalid_request;
        return;
    }

    m_connected = false;
    if( m_on_disconnected )
        m_on_disconnected();

    m_service.post([this]() { m_socket.close(); });
}

void client::send_raw( const std::string &cmd_str )
{
    if( !m_connected )
    {
        m_lasterror = error_code::invalid_request;

        m_service.post( std::bind( &client::send_raw,
                                    shared_from_this(), cmd_str ) );
    }
    else
    {
        m_lasterror = error_code::success;

        m_buf_write.consume( m_buf_write.size() );
        std::ostream out( &m_buf_write );
        out << cmd_str << "\r\n";

        m_service.post( std::bind( &client::loop, shared_from_this(),
                                    system_error_code(), 0 ) );
    }
}

void client::action( const std::string &destination, const std::string &message )
{
    if( destination.empty() || message.empty() )
    {
        m_lasterror = error_code::invalid_request;
        m_service.post( std::bind( &client::loop, shared_from_this(),
                                    system_error_code(), 0 ) );
        return;
    }

    send_raw("PRIVMSG "+ destination +" :\x01"+"ACTION "+ message +"\x01");
}

void client::ctcp_request( const std::string &nickname, const std::string &request )
{
    if( nickname.empty() || request.empty() )
    {
        m_lasterror = error_code::invalid_request;
        m_service.post( std::bind( &client::loop, shared_from_this(),
                                    system_error_code(), 0 ) );
        return;
    }
    send_raw("PRIVMSG "+ nickname +" :\x01"+ request +"\x01");
}

void client::ctcp_reply( const std::string &nickname, const std::string &reply )
{
    if( nickname.empty() || reply.empty() )
    {
        m_lasterror = error_code::invalid_request;
        m_service.post( std::bind( &client::loop, shared_from_this(),
                                    system_error_code(), 0 ) );
        return;
    }
    send_raw("NOTICE "+ nickname +" :\x01"+ reply +"\x01");
}

void client::invite( const std::string &nickname, const std::string &channel )
{
    if( nickname.empty() || channel.empty() )
    {
        m_lasterror = error_code::invalid_request;
        m_service.post( std::bind( &client::loop, shared_from_this(),
                                    system_error_code(), 0 ) );
        return;
    }

    send_raw("INVITE "+ nickname +" "+ channel);
}

void client::join( const std::string &channel )
{
    if( channel.empty() )
    {
        m_lasterror = error_code::invalid_request;
        m_service.post( std::bind( &client::loop, shared_from_this(),
                                       system_error_code(), 0 ) );
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
        m_service.post( std::bind( &client::loop, shared_from_this(),
                                       system_error_code(), 0 ) );
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
        m_service.post( std::bind( &client::loop, shared_from_this(),
                                       system_error_code(), 0 ) );
        return;
    }

    send_raw("NAMES "+ channel);
}

std::string client::nickname_from( const std::string &hostmask ) const
{
    if( hostmask.empty() )
        return std::string();

    std::size_t found = hostmask.find('!');
    if( found != std::string::npos )
        return hostmask.substr( 0, found );

    return std::string();
}

void client::notice( const std::string &destination, const std::string &message )
{
    if( destination.empty() || message.empty() )
    {
        m_lasterror = error_code::invalid_request;
        m_service.post( std::bind( &client::loop, shared_from_this(),
                                       system_error_code(), 0 ) );
        return;
    }

    send_raw("NOTICE "+ destination +" :"+ message);
}

void client::part( const std::string &channel )
{
    if( channel.empty() )
    {
        m_lasterror = error_code::invalid_request;
        m_service.post( std::bind( &client::loop, shared_from_this(),
                                       system_error_code(), 0 ) );
        return;
    }

    send_raw("PART "+ channel);
}

void client::privmsg( const std::string &destination, const std::string &message )
{
    if( destination.empty() || message.empty() )
    {
        m_lasterror = error_code::invalid_request;
        m_service.post( std::bind( &client::loop, shared_from_this(),
                                       system_error_code(), 0 ) );
        return;
    }

    send_raw("PRIVMSG "+ destination +" :"+ message);
}

void client::quit( const std::string &reason )
{
    std::string cmd_str = reason.empty() ? "QUIT" : "QUIT :"+ reason;
    send_raw( cmd_str );
}

void client::topic( const std::string &channel, const std::string &topic )
{
    if( channel.empty() )
    {
        m_lasterror = error_code::invalid_request;
        m_service.post( std::bind( &client::loop, shared_from_this(),
                                       system_error_code(), 0 ) );
        return;
    }

    std::string cmd_str;

    if( topic.empty() )
        cmd_str = "TOPIC "+ channel;
    else
        cmd_str = "TOPIC "+ channel +" :"+ topic;

    send_raw( cmd_str );
}

std::string client::version() const
{
    return "VERSION irc::client by Andrea Zanellato v0.1";
}

} // namespace irc

#endif // IRC_IMPL_CLIENT_HPP
