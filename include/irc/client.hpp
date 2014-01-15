/*
    Name:        irc/client.hpp
    Purpose:     IRC client interface
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/01/09
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_CLIENT_HPP
#define IRC_CLIENT_HPP

#include <functional>
#include <memory>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/system/error_code.hpp>

#include "irc/error.hpp"
#include "irc/numeric.hpp"

namespace ph = std::placeholders;

/** Main namespace */
namespace irc {

typedef boost::system::error_code      system_error_code;
typedef boost::asio::io_service        io_service;
typedef boost::asio::ip::tcp::resolver resolver;
typedef boost::asio::ip::tcp::socket   socket;
typedef boost::asio::streambuf         streambuf;

const int max_params = 15; /**< RFC 2812: maximum parameters allowed */
/**
    @class client

    IRC Client class.
*/
class client: public std::enable_shared_from_this< client >
            , public boost::asio::coroutine
            , boost::noncopyable
{
public:
/** Shared client pointer */
    typedef std::shared_ptr< client > ptr;
/**
    Static constructor.
    @param io_service Reference to the ASIO io_service controller.
    @return Shared pointer to a new client object.
*/
    static ptr create( io_service &io_service );

/** Destructor. */
    ~client();
/**
    Connects to an irc server via IPV4.
    @param hostname Server hostname to connect to.
    @param port     Server port to connect to.
    @param nickname Nick name for the client connection.
    @param username User name for the client connection.
    @param realname Real name for the client connection.
    @param srv_pwrd Password to login to a server that requires a key.
*/
    void connect( const std::string &hostname,
                  const std::string &port     = "6667",
                  const std::string &nickname = "nobody",
                  const std::string &username = "nobody",
                  const std::string &realname = "noname",
                  const std::string &srv_pwrd = std::string() );
/**
    Returns the connection state. 
    @return @true if connected, @false otherwise.
*/
    bool connected() const { return m_connected; }
/**
    Disconnects the active connection with the irc server.
*/
    void disconnect();
/**
    Sends a raw cmd_str to the server.
    @param cmd_str The cmd_str string to send.
*/
    void send_raw( const std::string &cmd_str );
/**
    An user action, the typical /me cmd_str.
    @param destination A channel or nickname target to send the action message.
    @param message     The action message.
*/
    void action( const std::string &destination, const std::string &message );
/**
    Sends a CTCP request.
    @param nickname The target nickname to send the request to.
    @param request  The CTCP request string.
*/
    void ctcp_request( const std::string &nickname, const std::string &request );
/**
    Sends a CTCP reply.
    @param nickname The target nickname to send the reply to.
    @param reply    The CTCP reply string.
*/
    void ctcp_reply( const std::string &nickname, const std::string &reply );
/**
    Sends an invitation to some user to join a channel.
    @param nickname The user to invite.
    @param channel  The channel to join for the invited user.
*/
    void invite( const std::string &nickname, const std::string &channel );
/**
    Joins a channel.
    @param channel The channel to join.
*/
    void join( const std::string &channel );
/**
    Kick someone from a channel.
    @param nickname The user to kick off.
    @param channel  The channel from which the user should be kicked out.
    @param reason   The kick reason (optional).
*/
    void kick( const std::string &nickname, const std::string &channel,
               const std::string &reason = std::string() );
/**
    Requests a list of channel details (name, user count and topic) from the server.
    @param channels A comma separated list of channels from which to list details.
                    If not specified, all server's channels will be listed.
*/
    void list( const std::string &channels = std::string() );
/**
    Requests the channel's user list.
    @param channel The channel where users are in.
*/
    void names( const std::string &channel );
/**
    Returns a nickname string from a hostmask.
    @param hostmask The hostmask to convert from.
    @return A nickname string.
*/
    std::string nickname_from( const std::string &hostmask ) const;
/**
    Sends a notice message to an user or channel.
    @param destination The user or channel where to send the message.
    @param message     The message to send as notice.
*/
    void notice( const std::string &destination, const std::string &message );
/**
    Leaves a specified channel.
    @param channel The channel to leave.
*/
    void part( const std::string &channel );
/**
    Sends a message to an user or channel.
    @param destination The user or channel where to send the message.
    @param message     The message to send.
*/
    void privmsg( const std::string &destination, const std::string &message );
/**
    Quits the client connection.
    @param reason The quit reason (optional).
*/
    void quit( const std::string &reason = std::string() );
/**
    Requests or sets a channel topic.
    @param channel The channel where to get/set the topic.
    @param topic   The new topic to set. If not specified,
                   the current topic is returned.
*/
    void topic( const std::string &channel, const std::string &topic = std::string() );
/**
    Returns the irc client version.
    @return The irc client version.
*/
    std::string version() const;
/**
    Signal fired when a message was sent to a channel.
    @param func The function to call back.
*/
    void on_channel_msg( std::function<void(const std::string &,
                                            const std::string &,
                                            const std::string &)> func )
    {
        m_on_chanmsg = func;
    }
/**
    Signal fired when the connection was enstablished.
    @param func The function to call back.
*/
    void on_connected( std::function<void()> func )
    {
        m_on_connected = func;
    }
/**
    Signal fired when disconnected from the server.
    @param func The function to call back.
*/
    void on_disconnected( std::function<void()> func )
    {
        m_on_disconnected = func;
    }
/**
    Signal fired when a numeric reply is sent from the IRC server.
    @param func The function to call back.
*/
    void on_numeric_reply( std::function<void(reply_code)> func )
    {
        m_on_numeric = func;
    }

private:
    explicit client( io_service &io_service )
    :   m_service(io_service),
        m_socket(io_service),
        m_connected(false),
        m_lasterror(error_code::success)
    {
        m_buf_read.prepare(512);
        m_buf_write.prepare(512);
    }

    client() = delete;

    void loop( const system_error_code &ec, size_t /*bytes*/ )
    {
        if( !ec ) { BOOST_ASIO_CORO_REENTER( this ) { for( ;; )
        {
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
        }}}
    }

    void handle_connect( const system_error_code &ec )
    {
        if( !ec && !m_connected )
        {
            if( m_on_connected )
                m_on_connected();

            std::ostream out( &m_buf_write );
            std::string
            nick = boost::str( boost::format("NICK %1%\r\n") % m_nickname ),
            user = boost::str( boost::format
                ("USER %1% unknown unknown :%2%\r\n") % m_username % m_realname );

            out << nick << user;

            m_service.post( std::bind( &client::loop,
                                           shared_from_this(), ec, 0 ) );
        }
    }

    void handle_read()
    {
        m_connected = true;
        std::istream in( &m_buf_read );
        std::string  line;
        std::getline( in, line );
        if( line.empty() )
        {
            m_service.post( std::bind( &client::loop, shared_from_this(),
                                           system_error_code(), 0 ) );
            return;
        }

        // Remove carriage return
        line.pop_back();

#ifdef IRC_DEBUG
        std::cout << line << '\n';
#endif
        // Extract prefix
        std::string sender;
        std::size_t found = line.find_first_of(' ');
        if( line.find(':') != std::string::npos && found != std::string::npos )
        {
            line.replace( 0, 1, "" );
            sender = line.substr( 0, found - 1 );
            line.replace( 0, found, "" );
        }

        // Extract command
        std::string cmd_str;
        int cmd_num = 0;
        found = line.find(' ');
        if( found != std::string::npos )
        {
            cmd_str = line.substr( 0, found );

            bool is_numeric = std::find_if( cmd_str.begin(), cmd_str.end(),
                [](char ch) { return !std::isdigit(ch); }) == cmd_str.end();

            if( is_numeric )
            {
                cmd_num = std::atoi( cmd_str.c_str() );
                reply_code rplcode = static_cast<reply_code>( cmd_num );
                if( m_on_numeric )
                    m_on_numeric( rplcode );
#ifdef IRC_DEBUG
                std::cout << "# command:" << cmd_num << '\n';
#endif
            }
#ifdef IRC_DEBUG
            else { std::cout << "# command:" << cmd_str << '\n'; }
#endif
            line.replace( 0, found + 1, "" );
        }

        // Extract recipient
        std::string recipient;
        found = line.find_first_of(' ');
        if( found != std::string::npos )
        {
            recipient = line.substr( 0, found );
            line.replace( 0, found, "" );
        }

        // Extract last param
        std::string last_param;
        found = line.find(" :");
        if( found != std::string::npos )
        {
            last_param = line.substr( found + 2, line.size() );
            line.replace( found, line.size(), "" );
        }

        // Split line to params
        std::vector<std::string> params;
        if( !line.empty() )
            boost::split( params, line, boost::is_any_of("\t ") );

        if( !last_param.empty() )
            params.push_back(last_param);

        // Extract params
        std::string content;
        if( params.size() )
        {
            content = params[0];
            for( size_t i = 1; i < params.size(); ++i )
                content += " " + params[i];
        }

        // Handle ping TODO: Swap recipient with sender for ERROR JOIN...
        if( cmd_str == "PING" && !recipient.empty() )
        {
            std::swap( recipient, sender );
            if( sender.find(':') != std::string::npos )
                sender.replace( 0, 1, "" );

            m_service.dispatch( std::bind( &client::pong,
                                               shared_from_this(), sender ) );
        }
        else if( cmd_str == "PRIVMSG" && !content.empty() )
        {
            std::string sender_nick = nickname_from( sender );
            if( content.find(':') != std::string::npos )
                content.replace( 0, 1, "" );

            std::size_t msg_len = content.size();

            // CTCP requests starts/ends with 0x01
            if( content[0] == 0x01 && content[msg_len - 1] == 0x01 )
            {
                msg_len -= 2;
                std::string ctcp_str = content.substr( 1, msg_len );

                if( ctcp_str.find("ACTION") != std::string::npos )
                {
                    if( m_on_action )
                        m_on_action(ctcp_str);
                }
                else if( ctcp_str.find("DCC") != std::string::npos )
                {
                    if( m_on_dcc_req )
                        m_on_dcc_req(ctcp_str);
                }
                else if( ctcp_str.find("FINGER") != std::string::npos )
                {
                    
                }
                else if( ctcp_str.find("PING") != std::string::npos )
                {
                    if( !sender_nick.empty() )
                        ctcp_reply( sender_nick, ctcp_str );
                }
                else if( ctcp_str.find("TIME") != std::string::npos )
                {
                    //
                }
                else if( ctcp_str.find("VERSION") != std::string::npos )
                {
                    if( m_on_version )
                    {
                        m_on_version();
                    }
                    else
                    {
                        if( !sender_nick.empty() )
                            ctcp_reply( sender_nick, version() );
                    }
                }
            }
            else if( recipient.find(m_nickname) != std::string::npos )
            {
                if( m_on_privmsg )
                    m_on_privmsg( sender_nick, sender, content );
            }
            else
            {
                if( m_on_chanmsg )
                    m_on_chanmsg( sender_nick, recipient, content );
            }
        }
        else if( cmd_str == "NOTICE" && !content.empty() )
        {
            std::size_t msg_len  = content.size();
            std::string sender_nick = nickname_from( sender );

            // CTCP
            if( content[0] == 0x01 && content[msg_len - 1] == 0x01 )
            {
                msg_len -= 2;
                std::string ctcp_str = content.substr( 1, msg_len );
            }
            else if( recipient.find(m_nickname) != std::string::npos )
            {
                if( m_on_privntc )
                    m_on_privntc( sender_nick, recipient, content );
            }
            else
            {
                if( m_on_channtc )
                    m_on_channtc( sender_nick, recipient, content );
            }
        }
        else if(cmd_str == "INVITE")
        {
            if( m_on_invite )
            {
                std::string sender_nick = nickname_from( sender );
                m_on_invite( sender_nick, recipient, content );
            }
        }
        else if(cmd_str == "KILL")
        {
            ;// ignore this event, not all servers generate this.
        }
        else // Unknown cmd_str
        {
            if( m_on_unknown )
                m_on_unknown();
        }
#ifdef IRC_DEBUG
        std::cout << "# message:" << content   << '\n'
//                << "# command:" << cmd_str   << '\n'
                  << "# from   :" << sender    << '\n'
                  << "# to     :" << recipient << '\n';
#endif
        m_service.post( std::bind( &client::loop, shared_from_this(),
                                       system_error_code(), 0 ) );
    }

    void pong( const std::string &sender ) { send_raw("PONG " + sender); }

    void handle_ctcp( const std::string &sender )
    {
        
    }

    io_service &m_service;
    socket      m_socket;
    bool        m_connected;
    std::string m_nickname,
                m_username,
                m_realname;
    streambuf   m_buf_read,
                m_buf_write;
    error_code  m_lasterror;

    std::function<void()> m_on_unknown;
    std::function<void(const std::string &,
                       const std::string &,
                       const std::string &)> m_on_invite;
    std::function<void(const std::string &,
                       const std::string &,
                       const std::string &)> m_on_channtc;
    std::function<void(const std::string &,
                       const std::string &,
                       const std::string &)> m_on_privntc;
    std::function<void(const std::string &,
                       const std::string &,
                       const std::string &)> m_on_chanmsg;
    std::function<void(const std::string &,
                       const std::string &,
                       const std::string &)> m_on_privmsg;
    std::function<void(const std::string &)> m_on_action;
    std::function<void(const std::string &)> m_on_dcc_req;
    std::function<void(reply_code)>          m_on_numeric;
    std::function<void()>                    m_on_connected;
    std::function<void()>                    m_on_disconnected;
    std::function<void()>                    m_on_version;
};

} // namespace irc

#ifdef IRC_CLIENT_HEADER_ONLY
    #include "irc/impl/client.ipp"
#endif

#endif // IRC_CLIENT_HPP
