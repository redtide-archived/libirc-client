/*
    Name:        example/console.cpp
    Purpose:     IRC client console example
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/01/08
    Licence:     Boost Software License, Version 1.0
*/
#include "handlers.hpp"

#include <irc/ctcp/dcc/session.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/signals2/signal.hpp>

#include <exception>
#include <functional>
#include <iostream>
#include <thread>
#include <vector>

namespace asio    = boost::asio;
namespace dcc     = irc::ctcp::dcc;
namespace ph      = std::placeholders;
namespace signals = boost::signals2;

int main( int argc, char **argv )
{
    using connections = std::vector<signals::connection>;
    using dccsessions = std::vector<dcc::session::ptr>;

    try
    {
        if( argc < 4 )
        {
            std::cerr << "Usage: irc_client <host> <port> <nickname> [#channel]\n";
            return 1;
        }

        asio::io_service ios;
        irc::client::ptr c = irc::client::create(ios);

        c->connect( argv[1], argv[2], argv[3] );

        connections conns;
        dccsessions dccs;

        conns.push_back( c->on_connected( &on_connected ) );
        conns.push_back( c->on_disconnected( &on_disconnect ) );

        conns.push_back( c->on_channel_message( &on_chan_message ) );
        conns.push_back( c->on_channel_mode( &on_chan_mode ) );
        conns.push_back( c->on_channel_notice( &on_chan_notice ) );
        conns.push_back( c->on_ctcp_request(
                            std::bind( &on_ctcp_request, c,
                                        ph::_1, ph::_2, ph::_3 ) ) );
        conns.push_back(
            c->on_dcc_request(
            [&ios](const std::string &address, const std::string &port,
            irc::ctcp::dcc::command type, const std::string &argument, long size)
            {
                ctcp::dcc::session::ptr dcc_session =
                    ctcp::dcc::session::make_shared( ios );

                dcc_session->connect( address, port, type, argument, size );
            })
        );
        conns.push_back( c->on_invite( &on_invite ) );
        conns.push_back( c->on_join( &on_join ) );
        conns.push_back( c->on_kick( &on_kick ) );
        conns.push_back( c->on_nick( &on_nick ) );
        conns.push_back( c->on_part( &on_part ) );
        conns.push_back( c->on_ping( std::bind(&on_ping, c, ph::_1) ) );
        conns.push_back( c->on_private_message( &on_privmsg ) );
        conns.push_back( c->on_private_notice( &on_notice ) );
        conns.push_back( c->on_quit( &on_quit ) );
        conns.push_back( c->on_topic( &on_topic ) );
        conns.push_back( c->on_unknown_command( &on_unknown ) );
        conns.push_back( c->on_user_mode( &on_user_mode ) );
        conns.push_back( c->on_numeric_command( &on_numeric ) );

        std::string channel;
        if( argc == 5 )
            channel = argv[4];

        // doesn't work because no more send_raw -> io_service.post 
//      c->join( channel );

        std::thread t([&ios](){ ios.run(); });

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
            else if( msg.find("/chat") != std::string::npos && has_txt )
            {
                c->dcc_chat(txt);
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

        for( auto conn : conns )
            conn.disconnect();

    }
    catch( std::exception &e )
    {
        std::cerr << "Exception: " << e.what() << '\n';
    }

    return 0;
}
