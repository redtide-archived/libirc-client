/*
    Name:        example/console.cpp
    Purpose:     IRC client console example
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/01/08
    Licence:     Boost Software License, Version 1.0
*/
#include "handlers.hpp"

#include <irc/dcc/request.hpp>
#include <irc/dcc/chat_client.hpp>
//#include <irc/dcc/file_client.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/signals2/signal.hpp>

#include <exception>
#include <functional>
#include <iostream>
#include <thread>
#include <vector>

bool is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(), s.end(),
        [](char c) { return !std::isdigit(c); }) == s.end();
}

int main( int argc, char **argv )
{
    namespace asio    = boost::asio;
    namespace dcc     = irc::dcc;
    namespace ph      = std::placeholders;
    namespace signals = boost::signals2;

    using connections = std::vector<signals::connection>;
    using dccsessions = std::vector<dcc::chat_client::ptr>;

    try
    {
        if( argc < 4 )
        {
            std::cerr << "Usage: irc_client <host> <port> <nickname> [#channel]\n";
            return 1;
        }

        asio::io_service ios;
        irc::client::ptr c = irc::client::create(ios, argv[1], argv[2], argv[3]);
        c->connect();

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
        conns.push_back( c->on_dcc_request( &on_dcc_req ) );
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

//      this doesn't work because no more send_raw -> io_service.post 
//      c->join( channel );

        std::thread t([&ios](){ ios.run(); });

        char raw_msg[512];
        while( std::cin.getline( raw_msg, sizeof(raw_msg) ) )
        {
            std::string msg( raw_msg );
            std::vector<std::string> tokens;
            boost::split( tokens, msg, boost::is_any_of(" ") );

            if( !tokens.empty() )
            {
                if( tokens[0] == "/q" )
                {
                    if( tokens.size() > 1 )
                        c->quit(tokens[1]);
                    else
                        c->quit();

                    break;
                }
                else if( tokens[0] == "/chat" && tokens.size() == 2 )
                {
                    // DCC CHAT offer
                    // /chat <nickname>
/*
                    irc::prefix pfx{tokens[1], std::string{}, std::string{}};
                    dcc::chat_server::ptr dcc_session =
                        dcc::session::make_shared( ios, dcc::command::chat, pfx,
                                                    c->address(), "0", true);
                    if( dcc_session )
                    {
                        dccs.push_back( dcc_session );
                        conns.push_back(dcc_session->on_dcc_message(
                                                    &on_dcc_msg));
                        conns.push_back(dcc_session->on_connected(
                                                    &on_dcc_connected));
                        conns.push_back(dcc_session->on_disconnected(
                                                    &on_dcc_disconnected));
                        dcc_session->connect();
                        c->dcc_chat( tokens[1], dcc_session->port() );
                    }
*/
                }
                else if( tokens[0] == "/chat" && tokens.size() == 4 )
                {
                    // DCC CHAT accept
                    // /chat <nickname> <address> <port>
                    irc::prefix pfx{tokens[1], std::string{},
                                    std::string{}};

                    dcc::chat_client::ptr dcc_session =
                        dcc::chat_client::make_shared(ios,pfx,tokens[2],tokens[3]);

                    if( dcc_session )
                    {
                        dccs.push_back( dcc_session );
                        conns.push_back(dcc_session->on_dcc_message(
                                                    &on_dcc_msg));
                        conns.push_back(dcc_session->on_connected(
                                                    &on_dcc_connected));
                        conns.push_back(dcc_session->on_disconnected(
                                                    &on_dcc_disconnected));
                        dcc_session->connect();
                    }
                }
                else if( tokens[0] == "/send" && tokens.size() == 3 )
                {
                    // Offer a DCC SEND file to user
                    // /send <nickname> <full_file_path>
/*
                    irc::prefix pfx{tokens[1], std::string{}, std::string{}};
                    dcc::session::ptr dcc_session =
                        dcc::session::make_shared(ios, pfx, c->address(), "0",
                                                    tokens[2], true);
                    dccs.push_back( dcc_session );
                    conns.push_back(dcc_session->on_dcc_send(
                                                &on_dcc_send));
                    conns.push_back(dcc_session->on_connected(
                                                &on_dcc_connected));
                    conns.push_back(dcc_session->on_disconnected(
                                                &on_dcc_disconnected));
                    dcc_session->connect();

                    c->dcc_send( tokens[1], dcc_session->port(), tokens[2] );*/
                }
                else if( tokens[0] == "/ping" && tokens.size() > 1 )
                {
                    // TODO c->ping( tokens[1] ) ?
                    c->ctcp_request( tokens[1], "PING" );
                }
                else if( tokens[0] == "/msg" && tokens.size() > 2 )
                {
                    std::string text;
                    for( std::size_t i = 2; i < tokens.size(); ++i )
                    {
                        text += tokens[i];
                        if( i != tokens.size() - 1 )
                            text += ' ';
                    }
                    c->privmsg( tokens[1], text );
                }
                else if( tokens[0] == "/dm" && tokens.size() > 2 )
                {
                    // DCC Message.
                    // /dm <dcc_id> <message>
                    if( !is_number( tokens[1] ) )
                        continue;

                    size_t id = std::stoi(tokens[1]);
                    if( id >= dccs.size() )
                        continue;

                    std::string text;
                    for( std::size_t i = 2; i < tokens.size(); ++i )
                    {
                        text += tokens[i];
                        if( i != tokens.size() - 1 )
                            text += ' ';
                    }
                    dccs[id]->write(text);
                }
                else if( tokens[0] == "/raw" && tokens.size() > 2 )
                {
                    std::string text;
                    for( std::size_t i = 1; i < tokens.size(); ++i )
                    {
                        text += tokens[i];
                        if( i != tokens.size() - 1 )
                            text += ' ';
                    }
                    c->send_raw( text );
                }
                else if( tokens[0] == "/me" && tokens.size() > 1 )
                {
                    c->action( channel, tokens[1] );
                }
                else if( tokens[0] == "/mode" && tokens.size() > 1 )
                {
                    c->set_channel_mode( channel, tokens[1] );
                }
                else if( tokens[0] == "/umode" && tokens.size() > 1 )
                {
                    c->set_mode( tokens[1] );
                }
                else if( tokens[0] == "/nick" && tokens.size() > 1 )
                {
                    c->nick( tokens[1] );
                }
                else if( tokens[0] == "/j" && tokens.size() > 1 )
                {
                    c->join( tokens[1] );
                }
                else if( tokens.size() > 1 )
                {
                    c->privmsg( channel, tokens[1] );
                }
            }
        }

        for( auto dcc : dccs )
        {
            if( dcc )
                dcc->disconnect();
        }

        for( auto conn : conns )
            conn.disconnect();

        t.join();
    }
    catch( std::exception &e )
    {
        std::cerr << "Exception: " << e.what() << '\n';
    }

    return 0;
}
