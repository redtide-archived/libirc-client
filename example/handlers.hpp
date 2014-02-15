#include <irc/client.hpp>
#include <irc/command.hpp>
#include <irc/ctcp/command.hpp>
#include <irc/dcc/command.hpp>
#include <irc/dcc/request.hpp>

#include <boost/asio.hpp>

#include <ctime>
#include <cstdio>
#include <iostream>
#include <thread>
#include <type_traits>

namespace ctcp = irc::ctcp;

template <typename Enumeration>
auto as_integer(Enumeration const value)
    -> typename std::underlying_type<Enumeration>::type
{
    return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}
void on_connected()
{
    std::cout << "################### Connected ###################\n";
}
void on_disconnect()
{
    std::cout << "################### Disconnected ###################\n";
}
void on_chan_notice( const irc::message &msg )
{
    if( !msg.prefix.nickname.empty() ) // from user
        std::cout << msg.params[0] <<" *"<<
            msg.prefix.nickname <<"* "<< msg.params[1] <<'\n';
    else // from server
        std::cout << msg.params[0] <<' '<< msg.params[1] <<'\n';
}
void on_notice( const irc::message &msg )
{
    if( !msg.prefix.nickname.empty() ) // from user
        std::cout <<'*'<<
            msg.prefix.nickname <<"* "<< msg.params[1] <<'\n';
    else // from server
        std::cout << msg.params[0] <<' '<< msg.params[1] <<'\n';
}
void on_chan_message( const irc::message &msg )
{
    std::cout << msg.params[0] <<" <"<<
            msg.prefix.nickname <<"> "<< msg.params[1] <<'\n';
}
void on_privmsg( const irc::message &msg )
{
    std::cout <<"PM <"<< msg.prefix.nickname <<"> "<< msg.params[1] <<'\n';
}
void on_chan_mode( const irc::message &msg )
{
    std::cout << msg.params[0] <<": "<< msg.prefix.nickname
              <<" has set mode "<< msg.params[1] <<" "<< msg.params[2] <<'\n';
}
void on_dcc_req( const irc::prefix &pfx, const irc::dcc::request &req )
{
    std::string sType;
    if( req.type == irc::dcc::command::chat )
        sType = "CHAT";
    else if( req.type == irc::dcc::command::send )
        sType = "SEND";
    else
        return;

    std::cout <<"DCC "<< sType <<
                " request from "<< req.address <<
                " port: "       << req.port <<
                " argument: "   << req.argument <<
                " with size: "  << req.size <<'\n';
}
void on_dcc_msg( const irc::prefix &pfx, const std::string &msg )
{
    std::cout <<"DCC <"<< pfx.nickname << "> "<< msg << '\n';
}
void on_dcc_send( const irc::prefix &pfx, size_t filesize )
{
    ;
}
void on_dcc_connected()
{
    std::cout <<"DCC connected."<< '\n';
}
void on_dcc_disconnected()
{
    std::cout <<"DCC disconnected."<< '\n';
}
void on_user_mode( const irc::message &msg )
{
    std::cout <<"(i) "<< msg.params[0] <<" has set mode "<< msg.params[1] <<'\n';
}
void on_numeric( const irc::message &msg )
{
#ifdef IRC_DEBUG
    std::cout <<"#prefix :"<< msg.prefix <<'\n';
    std::cout <<"#command:"<< as_integer(msg.command) <<'\n';
    std::size_t i = 0;
    for( std::vector<std::string>::const_iterator it = msg.params.begin();
                                                  it!= msg.params.end(); ++it )
    {
        std::cout <<"#params["<< i <<"]:"<< *it <<'\n';
        ++i;
    }
#else
    std::string log = "* ";
    std::size_t i = 0;
    for( std::vector<std::string>::const_iterator it = msg.params.begin();
                                                  it!= msg.params.end(); ++it )
    {
        if( i > 0 )
            log += *it +' ';
        ++i;
    }
    std::cout << log << std::endl;
#endif
}
void on_invite( const irc::message &msg )
{
    std::cout <<"(i) "<< msg.prefix.nickname <<
                " invites you to channel "<< msg.params[1] <<'\n';
}
void on_join( const irc::message &msg )
{
    std::cout <<"--> "<< msg.prefix.nickname <<
                " has joined "<< msg.params[0] <<'\n';
}
void on_kick( const irc::message &msg )
{
    std::cout <<"<-- "<< msg.params[1] << " has been kicked from "<<
        msg.params[0] <<" by "<< msg.prefix.nickname;

    if( msg.params.size() > 2 )
        std::cout << ": "<< msg.params[2];

    std::cout << '\n';
}
void on_nick( const irc::message &msg )
{
    std::cout << msg.prefix.nickname <<
        " has changed his nickname to "<< msg.params[0];
}
void on_part( const irc::message &msg )
{
    std::cout <<"<-- "<< msg.prefix.nickname <<" has left "<<
        msg.params[0] <<'\n';
}
void on_ping( irc::client::ptr c, const irc::message &msg )
{
    c->pong( msg.params[0] );
    std::cout << "*PONG "<< msg.params[0] << "*\n";
}
void on_quit( const irc::message &msg )
{
    ;
}
void on_topic( const irc::message &msg )
{
    ;
}
void on_unknown( const irc::message &msg )
{
    ;
}
void on_ctcp_request( irc::client::ptr c, const irc::message &msg,
                      ctcp::command ctcp_cmd, const std::string &request )
{
    std::string ctcp_cmd_str;

    if( ctcp_cmd == ctcp::command::action )
    {
        std::cout << '*' << msg.prefix.nickname << "* " << request << '\n';
        return;
    }
    else if( ctcp_cmd == ctcp::command::clientinfo )
    {
        ctcp_cmd_str = "CLIENTINFO";
    }
    else if( ctcp_cmd == ctcp::command::dcc )
    {
        ctcp_cmd_str = "DCC";
        ;// m_on_dcc_req(  );
    }
    else if( ctcp_cmd == ctcp::command::finger )
    {
        ctcp_cmd_str = "FINGER";
        std::string finger = "FINGER: Pretty useless, obsolete CTCP request.";
        c->ctcp_reply( msg.prefix.nickname, finger );
    }
    else if( ctcp_cmd == ctcp::command::ping )
    {
        ctcp_cmd_str = "PING "+ request;
        c->ctcp_reply( msg.prefix.nickname, ctcp_cmd_str );
    }
    else if( ctcp_cmd == ctcp::command::source )
    {
        ctcp_cmd_str = "SOURCE";
        c->ctcp_reply( msg.prefix.nickname, "SOURCE: git@github.com:irclib/client.git" );
    }
    else if( ctcp_cmd == ctcp::command::time )
    {
        ctcp_cmd_str = "TIME";
        std::time_t time_now;
        std::time (&time_now);
        std::string time_reply = "TIME ";
        time_reply += std::ctime( &time_now );
        time_reply = time_reply.substr( 0, time_reply.length() -1 );
        c->ctcp_reply( msg.prefix.nickname, time_reply );
    }
    else if( ctcp_cmd == ctcp::command::version )
    {
        ctcp_cmd_str = "VERSION";
        c->ctcp_reply( msg.prefix.nickname, irc::version() );
    }
    else if( ctcp_cmd == ctcp::command::userinfo )
    {
        ctcp_cmd_str = "USERINFO";
        c->ctcp_reply( msg.prefix.nickname, "USERINFO: Just an irc::client example." );
    }

    std::cout << "CTCP "<< ctcp_cmd_str <<" request from "
              << msg.prefix.nickname << " replied.\n";
}
