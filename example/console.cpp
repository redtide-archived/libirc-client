/*
    Name:        example/console.cpp
    Purpose:     IRC client console example
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/01/08
    Licence:     Boost Software License, Version 1.0
*/
#include <iostream>
#include <functional>
#include <thread>
#include <irc/client.hpp>
#include <irc/numeric.hpp>

template <typename Enumeration>
auto as_integer(Enumeration const value)
    -> typename std::underlying_type<Enumeration>::type
{
    return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

void on_channel_msg( const std::string &origin,
                     const std::string &/*channel*/,
                     const std::string &message )
{
    std::cout << "<" << origin << "> " << message << '\n';
}

void on_numeric( irc::reply_code command )
{
    ;// std::cout << "# command:" << as_integer(command) << '\n';
}

void on_connected()
{
    std::cout << "################### Connected ###################\n";
}

void on_disconnect()
{
    std::cout << "################### Disconnected ###################\n";
}

namespace ph = std::placeholders;

int main( int argc, char **argv )
{
    try
    {
        if( argc != 5 )
        {
            std::cerr << "Usage: irc_client <host> <port> <nickname> <channel>\n";
            return 1;
        }

        irc::io_service  io_service;
        irc::client::ptr c = irc::client::create(io_service);

        c->connect( argv[1], argv[2], argv[3] );

        c->on_connected( &on_connected );
        c->on_disconnected( &on_disconnect );
//      c->on_numeric_reply( std::bind(&on_numeric, ph::_1) );
        c->on_channel_msg( std::bind(&on_channel_msg, ph::_1, ph::_2, ph::_3) );

        c->join( argv[4] );

        std::thread t([&io_service](){ io_service.run(); });

        char line[512];
        while( std::cin.getline( line, sizeof(line) ) )
        {
            std::string msg( line );
            if( msg.find("/quit") != std::string::npos )
            {
                std::size_t found  = msg.find_first_of(' ');
                std::size_t lenght = msg.size();
                std::string reason;
                if( found < lenght )
                    reason = msg.substr( found + 1, lenght );

                c->quit(reason);
                break;
            }
            else if( msg.find("/ping") != std::string::npos )
            {
                std::size_t found  = msg.find_first_of(' ');
                std::size_t lenght = msg.size();
                if( found < lenght )
                {
                    std::string nick = msg.substr( found + 1, lenght );
                    c->ctcp_request( nick, "PING" );
                }
            }
            else if( msg.find("/me") != std::string::npos )
            {
                std::size_t found  = msg.find_first_of(' ');
                std::size_t lenght = msg.size();
                if( found < lenght )
                {
                    std::string text = msg.substr( found + 1, lenght );
                    c->action( argv[4], text );
                }
            }
            else
            {
                c->privmsg( argv[4], msg );
            }
        }

        t.join();
    }
    catch( std::exception &e )
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
