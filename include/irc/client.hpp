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

namespace irc {

typedef boost::system::error_code      system_error_code;
typedef boost::asio::io_service        io_service_type;
typedef boost::asio::ip::tcp::resolver resolver_type;
typedef boost::asio::ip::tcp::socket   socket_type;
typedef boost::asio::streambuf         streambuf_type;
typedef irc::error_code                error_code;

const int max_params = 15; // RFC 2812

class client : public std::enable_shared_from_this< client >
             , public boost::asio::coroutine
             , boost::noncopyable
{
public:
    typedef std::shared_ptr< client > pointer;

    static pointer create( io_service_type &io_service )
    {
        pointer new_connection( new client( io_service ) );
        return new_connection;
    }

    void connect( const std::string &hostname,
                  const std::string &port     = "6667",
                  const std::string &nickname = "nobody",
                  const std::string &username = "nobody",
                  const std::string &realname = "noname",
                  const std::string &srv_pwrd = std::string() )
    {
        resolver_type           resolver( m_io_service );
        resolver_type::query    query( hostname, port );
        resolver_type::iterator endpoint_iter = resolver.resolve( query );

        m_nickname = nickname;
        m_username = username;
        m_realname = realname;

        boost::asio::async_connect( m_socket, endpoint_iter,
                                    std::bind( &client::handle_connect,
                                                shared_from_this(), ph::_1 ) );
    }

    bool connected() const { return m_started; }

    void disconnect()
    {
        if(!m_started)
        {
            m_lasterror = error_code::invalid_request;
            return;
        }

        m_started = false;
        if( m_on_disconnected )
            m_on_disconnected();

        m_io_service.post([this]() { m_socket.close(); });
    }

    void send_raw( const std::string &command )
    {
        if( !m_started )
        {
            m_lasterror = error_code::invalid_request;

            m_io_service.post( std::bind( &client::send_raw,
                                           shared_from_this(), command ) );
        }
        else
        {
            m_lasterror = error_code::success;

            m_buf_write.consume( m_buf_write.size() );
            std::ostream out( &m_buf_write );
            out << command << "\r\n";

            m_io_service.post( std::bind( &client::step, shared_from_this(),
                                           system_error_code(), 0 ) );
        }
    }

    void action( const std::string &destination, const std::string &message )
    {
        if( destination.empty() || message.empty() )
        {
            m_lasterror = error_code::invalid_request;
            m_io_service.post( std::bind( &client::step, shared_from_this(),
                                           system_error_code(), 0 ) );
            return;
        }

        send_raw("PRIVMSG "+ destination +" :\x01"+"ACTION "+ message +"\x01");
    }

    void ctcp_request( const std::string &nickname, const std::string &request )
    {
        if( nickname.empty() || request.empty() )
        {
            m_lasterror = error_code::invalid_request;
            m_io_service.post( std::bind( &client::step, shared_from_this(),
                                           system_error_code(), 0 ) );
            return;
        }
        send_raw("PRIVMSG "+ nickname +" :\x01"+ request +"\x01");
    }

    void ctcp_reply( const std::string &nickname, const std::string &reply )
    {
        if( nickname.empty() || reply.empty() )
        {
            m_lasterror = error_code::invalid_request;
            m_io_service.post( std::bind( &client::step, shared_from_this(),
                                           system_error_code(), 0 ) );
            return;
        }
        send_raw("NOTICE "+ nickname +" :\x01"+ reply +"\x01");
    }

    void invite( const std::string &nickname, const std::string &channel )
    {
        if( nickname.empty() || channel.empty() )
        {
            m_lasterror = error_code::invalid_request;
            m_io_service.post( std::bind( &client::step, shared_from_this(),
                                           system_error_code(), 0 ) );
            return;
        }

        send_raw("INVITE "+ nickname +" "+ channel);
    }

    void join( const std::string &channel )
    {
        if( channel.empty() )
        {
            m_lasterror = error_code::invalid_request;
            m_io_service.post( std::bind( &client::step, shared_from_this(),
                                           system_error_code(), 0 ) );
            return;
        }
        send_raw("JOIN "+ channel);
    }

    void kick( const std::string &nickname, const std::string &channel,
               const std::string &reason = std::string() )
    {
        if( nickname.empty() || channel.empty() )
        {
            m_lasterror = error_code::invalid_request;
            m_io_service.post( std::bind( &client::step, shared_from_this(),
                                           system_error_code(), 0 ) );
            return;
        }

        if( reason.empty() )
            send_raw("KICK "+ channel +" "+ nickname);
        else
            send_raw("KICK "+ channel +" "+ nickname +" :"+ reason);
    }

    void list( const std::string &channel = std::string() )
    {
        send_raw( channel.empty() ? "LIST" : "LIST "+ channel );
    }

    void names( const std::string &channel )
    {
        if( channel.empty() )
        {
            m_lasterror = error_code::invalid_request;
            m_io_service.post( std::bind( &client::step, shared_from_this(),
                                           system_error_code(), 0 ) );
            return;
        }

        send_raw("NAMES "+ channel);
    }

    std::string nickname_from( const std::string &hostmask ) const
    {
        if( hostmask.empty() )
            return std::string();

        std::size_t found = hostmask.find_first_of('!');
        if( found != std::string::npos )
            return hostmask.substr( 0, found );

        return std::string();
    }

    void notice( const std::string &nickname, const std::string &message )
    {
        if( nickname.empty() || message.empty() )
        {
            m_lasterror = error_code::invalid_request;
            m_io_service.post( std::bind( &client::step, shared_from_this(),
                                           system_error_code(), 0 ) );
            return;
        }

        send_raw("NOTICE "+ nickname +" :"+ message);
    }

    void part( const std::string &channel )
    {
        if( channel.empty() )
        {
            m_lasterror = error_code::invalid_request;
            m_io_service.post( std::bind( &client::step, shared_from_this(),
                                           system_error_code(), 0 ) );
            return;
        }

        send_raw("PART "+ channel);
    }

    void privmsg( const std::string &destination, const std::string &message )
    {
        if( destination.empty() || message.empty() )
        {
            m_lasterror = error_code::invalid_request;
            m_io_service.post( std::bind( &client::step, shared_from_this(),
                                           system_error_code(), 0 ) );
            return;
        }

        send_raw("PRIVMSG "+ destination +" :"+ message);
    }

    void quit( const std::string &reason = std::string() )
    {
        std::string command = reason.empty() ? "QUIT" : "QUIT :"+ reason;
        send_raw( command );
    }

    void topic( const std::string &channel, const std::string &topic = std::string() )
    {
        if( channel.empty() )
        {
            m_lasterror = error_code::invalid_request;
            m_io_service.post( std::bind( &client::step, shared_from_this(),
                                           system_error_code(), 0 ) );
            return;
        }

        std::string command;

        if( topic.empty() )
            command = "TOPIC "+ channel;
        else
            command = "TOPIC "+ channel +" :"+ topic;

        send_raw( command );
    }

    std::string version() const
    {
        return "VERSION irc::client by Andrea Zanellato v0.1";
    }

    void on_connected( std::function<void()> func )
    {
        m_on_connected = func;
    }

    void on_disconnected( std::function<void()> func )
    {
        m_on_disconnected = func;
    }

    void on_numeric_reply( std::function<void(reply_code)> func )
    {
        m_on_numeric = func;
    }

private:
    explicit client( io_service_type &io_service )
    :   m_io_service(io_service),
        m_socket(io_service),
        m_started(false),
        m_lasterror(error_code::success)
    {
    }

    void step( const system_error_code &ec, size_t /*bytes*/ )
    {
        if( !ec )
        {
            BOOST_ASIO_CORO_REENTER( this )
            {
                for( ;; )
                {
                    BOOST_ASIO_CORO_YIELD
                    async_read_until( m_socket, m_buf_read, "\r\n",
                                      std::bind( &client::step, shared_from_this(),
                                                  ph::_1, ph::_2 ) );
                    BOOST_ASIO_CORO_YIELD
                    async_write( m_socket, m_buf_write,
                                 std::bind( &client::step, shared_from_this(),
                                             ph::_1, ph::_2 ) );
                    BOOST_ASIO_CORO_YIELD
                    handle_read();
                }
            }
        }
        else
        {
            disconnect();
        }
    }

    void handle_connect( const system_error_code &ec )
    {
        if( !ec && !m_started )
        {
            if( m_on_connected )
                m_on_connected();

            std::ostream out( &m_buf_write );
            std::string
            nick = boost::str( boost::format("NICK %1%\n") % m_nickname ),
            user = boost::str( boost::format
                ("USER %1% unknown unknown :%2%\n") % m_username % m_realname );

            out << nick << user;

            m_io_service.post( std::bind( &client::step,
                                           shared_from_this(), ec, 0 ) );
        }
    }

    void handle_read()
    {
        std::istream in( &m_buf_read );
        std::string  line;
        std::getline( in, line );

        if( line.empty() )
        {
            m_io_service.post( std::bind( &client::step, shared_from_this(),
                                           system_error_code(), 0 ) );
            return;
        }

        line.pop_back(); // remove carriage return

#ifdef IRC_DEBUG
        std::cout << line << '\n';
#endif
        // Extract prefix
        std::string prefix;
        std::size_t start = line.find_first_of(':');
        std::size_t end   = line.find_first_of(' ');
        if( (start == 0) && (end != std::string::npos) && (end < line.size()) )
        {
            prefix = line.substr( 1, end - 1 );
            line.replace( 0, end + 1, "" );
#ifdef IRC_DEBUG
            std::cout << "# prefix=" << prefix << '\n';
#endif
        }

        // Extract command
        std::string command;
        int icommand = 0;
        end = line.find_first_of(' ');
        if( (end != std::string::npos) && (end < line.size()) )
        {
            command = line.substr( 0, end );

            bool is_numeric = std::find_if( command.begin(), command.end(),
                [](char ch) { return !std::isdigit(ch); }) == command.end();

            if( is_numeric )
            {
                icommand = std::atoi( command.c_str() );
                reply_code rplcode = static_cast<reply_code>( icommand );
                if( m_on_numeric )
                    m_on_numeric( rplcode );
            }
#ifdef IRC_DEBUG
            else { std::cout << "# command=" << command << '\n'; }
#endif
            line.replace( 0, end + 1, "" );
        }

        // Extract message
        std::string message;
        start = line.find_first_of(':');
        if( (start != std::string::npos) && (start < line.size()) )
        {
            message = line.substr( start + 1 );
            line.replace( start, line.size(), "" );
        }

        // Extract params - TODO: command 005 splitted incorrectly
        std::vector<std::string> params;
        boost::trim( line );
        boost::split( params, line, boost::is_any_of("\t "));
        if( !message.empty() )
            params.push_back( message );

#ifdef IRC_DEBUG
        size_t i = 0;
        for( auto it = std::begin(params); it != std::end(params); ++it )
        {
            std::cout << "# param[" << i << "]=" << *it << '\n';
            ++i;
        }
#endif
        if( command == "PING" && params.size() )
        {
            m_io_service.dispatch( std::bind( &client::handle_ping,
                                               shared_from_this(), params[0] ) );
        }
        else if( command == "PRIVMSG" && (params.size() > 1) )
        {
            std::size_t msg_len = params[1].size();

            // CTCP requests starts/ends with 0x01
            if( params[1][0] == 0x01 && params[1][msg_len - 1] == 0x01 )
            {
                msg_len -= 2;
                std::string ctcp_str = params[1].substr( 1, msg_len );
                std::string nickname = nickname_from( prefix );
                std::size_t end      = ctcp_str.find_first_of(' ');

                if( end != std::string::npos && msg_len > 2 )
                {
                    std::string cmd_str = ctcp_str.substr(0);
                }

                std::size_t found = ctcp_str.find("ACTION");
                if( found != std::string::npos )
                {
                    //
                }
                found = ctcp_str.find("DCC");
                if( found != std::string::npos )
                {
                    //
                    if( m_on_dcc_req )
                        m_on_dcc_req(ctcp_str);
                }
                found = ctcp_str.find("FINGER");
                if( found != std::string::npos )
                {
                    //
                }
                found = ctcp_str.find("PING");
                if( found != std::string::npos )
                {
                    ctcp_reply( nickname, ctcp_str );
                }
                found = ctcp_str.find("TIME");
                if( found != std::string::npos )
                {
                    //
                }
                found = ctcp_str.find("VERSION");
                if( found != std::string::npos )
                {
                    std::string ver = version();
                    if( !nickname.empty() )
                        ctcp_reply( nickname, ver );
                }
                if( found == std::string::npos )
                {
                    //
                }
            }
        }
        m_started = true;
        m_io_service.post( std::bind( &client::step, shared_from_this(),
                                       system_error_code(), 0 ) );
    }

    void handle_ping( const std::string &sender )
    {
        std::string command = "PONG " + sender + "\r\n";
        send_raw( command );
    }

    void handle_ctcp( const std::string &sender )
    {
        
    }

    io_service_type   &m_io_service;
    socket_type        m_socket;
    bool               m_started;
    std::string        m_nickname,
                       m_username,
                       m_realname;
    streambuf_type     m_buf_read,
                       m_buf_write;
    error_code         m_lasterror;

    std::function<void(const std::string &)> m_on_dcc_req;
    std::function<void(reply_code)>          m_on_numeric;
    std::function<void()>                    m_on_connected;
    std::function<void()>                    m_on_disconnected;
};

} // namespace irc

#endif // IRC_CLIENT_HPP
