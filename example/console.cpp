/*
    Name:        example/console.cpp
    Purpose:     IRC client console example
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/01/08
    Licence:     Boost Software License, Version 1.0
*/
#include "handlers.hpp"

#include <boost/algorithm/string.hpp>

#include <exception>
#include <functional>
#include <iostream>
#include <thread>
#include <vector>

namespace ph = std::placeholders;

int main( int argc, char **argv )
{
    try
    {
        if( argc < 4 )
        {
            std::cerr << "Usage: irc_client <host> <port> <nickname> [#channel]\n";
            return 1;
        }

        irc::io_service  io_service;
        irc::client::ptr c = irc::client::create(io_service);

        c->connect( argv[1], argv[2], argv[3] );
        c->connect_on_connected( &on_connected );
        c->connect_on_disconnected( &on_disconnect );

        c->connect_on_channel_message( &on_chan_message );
        c->connect_on_channel_mode( &on_chan_mode );
        c->connect_on_channel_notice( &on_chan_notice );
        c->connect_on_invite( &on_invite );
        c->connect_on_join( &on_join );
        c->connect_on_kick( &on_kick );
        c->connect_on_nick( &on_nick );
        c->connect_on_part( &on_part );
        c->connect_on_ping( std::bind(&on_ping, c, ph::_1) );
        c->connect_on_private_message( &on_privmsg );
        c->connect_on_private_notice( &on_notice );
        c->connect_on_quit( &on_quit );
        c->connect_on_topic( &on_topic );
        c->connect_on_unknown_command( &on_unknown );
        c->connect_on_user_mode( &on_user_mode );
        c->connect_on_numeric_command( &on_numeric );

        std::string channel;
        if( argc == 5 )
            channel = argv[4];

        // doesn't work because no more send_raw -> io_service.post 
        c->join( channel );

        std::thread t([&io_service](){ io_service.run(); });

        char raw_msg[512];
        while( std::cin.getline( raw_msg, sizeof(raw_msg) ) )
        {
            std::string msg( raw_msg );
            std::size_t len     = msg.size();
            std::size_t txt_pos = msg.find_first_of(' ');
            std::string txt     = msg.substr( txt_pos + 1, len );
            bool        has_txt = txt_pos != std::string::npos;

            if( msg.find("/q") != std::string::npos )
            {
                c->quit(txt);
                break;
            }
            else if( msg.find("/ping") != std::string::npos && has_txt )
            {
                c->ctcp_request(txt, "PING"); // TODO c->ping(txt) ?
            }
            else if( msg.find("/msg") != std::string::npos && has_txt )
            {
                std::vector<std::string> tokens;
                boost::split( tokens, msg, boost::is_any_of(" ") );
                if( tokens.size() > 2 )
                {
                    std::string target = tokens[1];
                    std::string text;
                    for( std::size_t i = 2; i < tokens.size(); ++i )
                    {
                        text += tokens[i] + ' ';
                    }
                    c->privmsg(target, text);
                }
            }
            else if( msg.find("/me") != std::string::npos && has_txt )
            {
                c->action(channel, txt);
            }
            else if( msg.find("/mode") != std::string::npos && has_txt )
            {
                c->set_channel_mode(channel, txt);
            }
            else if( msg.find("/umode") != std::string::npos && has_txt )
            {
                c->set_mode(txt);
            }
            else if( msg.find("/nick") != std::string::npos && has_txt )
            {
                c->nick(txt);
            }
            else if( msg.find("/j") != std::string::npos && has_txt )
            {
                c->join(txt);
            }
            else
            {
                c->privmsg(channel, txt);
            }
        }

        t.join();
    }
    catch( std::exception &e )
    {
        std::cerr << "Exception: " << e.what() << '\n';
    }

    return 0;
}
