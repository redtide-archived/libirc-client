#include <irc/client.hpp>
#include <irc/command.hpp>

#include <type_traits>
#include <iostream>

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
}/*
void on_action( const irc::message &msg )
{
    std::string user = msg.prefix.nickname;
    std::string chan = msg.params[0];
//  std::string what = msg.params[1];

//  std::cout << chan << " * " << user << " " << what << '\n';
}*/
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
