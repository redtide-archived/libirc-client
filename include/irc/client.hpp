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

#include "irc/command.hpp"
#include "irc/error.hpp"
#include "irc/message.hpp"
#include "irc/types.hpp"

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include <memory>

/** Main namespace */
namespace irc {

using system_error_code = boost::system::error_code;
using io_service        = boost::asio::io_service;
using resolver          = boost::asio::ip::tcp::resolver;
using socket            = boost::asio::ip::tcp::socket;
using streambuf         = boost::asio::streambuf;

const int max_params = 15; /**< RFC 2812: maximum parameters allowed */
/**
    Returns the irc client version.
    @return The irc client version.
*/
static std::string version();
/**
    Returns if the specified mask represents a channel.
    @param mask The channel mask to check.
    @return @true if it's a channel, @false otherwise.
    @see http://tools.ietf.org/html/rfc2811#section-2.1
*/
static bool is_channel( const std::string &mask );
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
    Sends a raw command to the server.
    @param command The command string to send.
*/
    void send_raw( const std::string &command );
/**
    An user (CTCP) action command,
    the typical '/me' in some IRC client applications.
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
    Changes your nickname.
    @param newnick The new nick to set.
*/
    void nick( const std::string &newnick );
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
    Replies to an IRC server PING command.
    Some servers sends PING commands to clients to check their status;
    a client must reply with a PONG message to keepalive to avoid disconnections.
    @param destination The IRC server to send the PONG reply.
*/
    void pong( const std::string &destination );
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

    void set_mode( const std::string &mode );

    void set_channel_mode( const std::string &channel, const std::string &mode );
/**
    Requests or sets a channel topic.
    @param channel The channel where to get/set the topic.
    @param topic   The new topic to set. If not specified,
                   the current topic is returned.
*/
    void topic( const std::string &channel, const std::string &topic = std::string() );

    std::string nickname() const { return m_nickname; }
    std::string username() const { return m_username; }
    std::string realname() const { return m_realname; }
/**
    Signal fired when an user sent a public message to a channel.
    @param func The function to call back.
    @return The connection object to disconnect from the signal.
*/
    template<typename Callback>
    signals::connection connect_on_channel_message( Callback&& func )
    { return m_on_chanmsg.connect( std::forward<Callback>(func) ); }
/**
    Signal fired when an user sent a public notice to a channel.
    @param func The function to call back.
    @return The connection object to disconnect from the signal.
*/
    template<typename Callback>
    signals::connection connect_on_channel_notice( Callback&& func )
    { return m_on_channtc.connect( std::forward<Callback>(func) ); }
/**
    Signal fired when an user sets the channel modes.
    @param func The function to call back.
    @return The connection object to disconnect from the signal.
*/
    template<typename Callback>
    signals::connection connect_on_channel_mode( Callback&& func )
    { return m_on_chanmode.connect( std::forward<Callback>(func) ); }
/**
    Signal fired when an user invited you to join a channel.
    @param func The function to call back.
    @return The connection object to disconnect from the signal.
*/
    template<typename Callback>
    signals::connection connect_on_invite( Callback&& func )
    { return m_on_invite.connect( std::forward<Callback>(func) ); }
/**
    Signal fired when an user joined a channel.
    @param func The function to call back.
    @return The connection object to disconnect from the signal.
*/
    template<typename Callback>
    signals::connection connect_on_join( Callback&& func )
    { return m_on_join.connect( std::forward<Callback>(func) ); }
/**
    Signal fired when an user kick someone out of a channel.
    @param func The function to call back.
    @return The connection object to disconnect from the signal.
*/
    template<typename Callback>
    signals::connection connect_on_kick( Callback&& func )
    { return m_on_kick.connect( std::forward<Callback>(func) ); }
/**
    Signal fired when an user changes his/her nick.
    @param func The function to call back.
    @return The connection object to disconnect from the signal.
*/
    template<typename Callback>
    signals::connection connect_on_nick( Callback&& func )
    { return m_on_nick.connect( std::forward<Callback>(func) ); }
/**
    Signal fired when an user sent a private notice to someone else.
    @param func The function to call back.
    @return The connection object to disconnect from the signal.
*/
    template<typename Callback>
    signals::connection connect_on_private_notice( Callback&& func )
    { return m_on_notice.connect( std::forward<Callback>(func) ); }
/**
    Signal fired when a numeric reply is sent from the IRC server.
    @param func The function to call back.
    @return The connection object to disconnect from the signal.
*/
    template<typename Callback>
    signals::connection connect_on_numeric_command( Callback&& func )
    { return m_on_numeric.connect( std::forward<Callback>(func) ); }
/**
    Signal fired when an user has left a channel.
    @param func The function to call back.
    @return The connection object to disconnect from the signal.
*/
    template<typename Callback>
    signals::connection connect_on_part( Callback&& func )
    { return m_on_part.connect( std::forward<Callback>(func) ); }
/**
    Signal fired when recieving a PING message from an IRC server.
    @param func The function to call back.
    @return The connection object to disconnect from the signal.
*/
    template<typename Callback>
    signals::connection connect_on_ping( Callback&& func )
    { return m_on_ping.connect( std::forward<Callback>(func) ); }
/**
    Signal fired when an user sent a private message to someone else.
    @param func The function to call back.
    @return The connection object to disconnect from the signal.
*/
    template<typename Callback>
    signals::connection connect_on_private_message( Callback&& func )
    { return m_on_privmsg.connect( std::forward<Callback>(func) ); }
/**
    Signal fired when an user quits IRC.
    @param func The function to call back.
    @return The connection object to disconnect from the signal.
*/
    template<typename Callback>
    signals::connection connect_on_quit( Callback&& func )
    { return m_on_quit.connect( std::forward<Callback>(func) ); }
/**
    Signal fired when an user changes a channel topic.
    @param func The function to call back.
    @return The connection object to disconnect from the signal.
*/
    template<typename Callback>
    signals::connection connect_on_topic( Callback&& func )
    { return m_on_topic.connect( std::forward<Callback>(func) ); }
/**
    Signal fired when your user mode changes.
    @param func The function to call back.
    @return The connection object to disconnect from the signal.
*/
    template<typename Callback>
    signals::connection connect_on_user_mode( Callback&& func )
    { return m_on_usermode.connect( std::forward<Callback>(func) ); }
/**
    Signal fired when an unsupported message command is sent.
    @param func The function to call back.
    @return The connection object to disconnect from the signal.
*/
    template<typename Callback>
    signals::connection connect_on_unknown_command( Callback&& func )
    { return m_on_unknown.connect( std::forward<Callback>(func) ); }
/**
    Signal fired when the connection was enstablished.
    @param func The function to call back.
    @return The connection object to disconnect from the signal.
*/
    template<typename Callback>
    signals::connection connect_on_connected( Callback&& func )
    { return m_on_connected.connect( std::forward<Callback>(func) ); }
/**
    Signal fired when disconnected from the server.
    @param func The function to call back.
    @return The connection object to disconnect from the signal.
*/
    template<typename Callback>
    signals::connection connect_on_disconnected( Callback&& func )
    { return m_on_disconnected.connect( std::forward<Callback>(func) ); }
/**
    Signal fired when recieving a CTCP request.
    @param func The function to call back.
    @return The connection object to disconnect from the signal.
*/
    template<typename Callback>
    signals::connection connect_on_ctcp_request( Callback&& func )
    { return m_on_ctcp_req.connect( std::forward<Callback>(func) ); }
/**
    Signal fired when recieving a CTCP reply.
    @param func The function to call back.
    @return The connection object to disconnect from the signal.
*/
    template<typename Callback>
    signals::connection connect_on_ctcp_reply( Callback&& func )
    { return m_on_ctcp_rep.connect( std::forward<Callback>(func) ); }

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

    void loop( const system_error_code &ec, size_t /*bytes*/ );
    bool parse( const std::string &raw_msg, message &msg );

    void handle_message( message &msg );
    void handle_read();

    io_service &m_service;
    socket      m_socket;
    bool        m_connected;
    std::string m_nickname,
                m_username,
                m_realname;
    streambuf   m_buf_read,
                m_buf_write;
    error_code  m_lasterror;

    sig_message m_on_chanmsg,
                m_on_channtc,
                m_on_chanmode,
                m_on_invite,
                m_on_join,
                m_on_kick,
                m_on_nick,
                m_on_notice,
                m_on_numeric,
                m_on_part,
                m_on_ping,
                m_on_privmsg,
                m_on_quit,
                m_on_topic,
                m_on_usermode,
                m_on_unknown;

    sig_void    m_on_connected,
                m_on_disconnected;

    sig_ctcp    m_on_ctcp_req,
                m_on_ctcp_rep;

//  sig_dcc_chat m_on_dcc_chat_req;
//  sig_dcc_send m_on_dcc_send_req;
};

} // namespace irc

#ifdef IRC_CLIENT_HEADER_ONLY
    #include "irc/impl/client.ipp"
    #include "irc/impl/loop.ipp"
#endif

#endif // IRC_CLIENT_HPP
