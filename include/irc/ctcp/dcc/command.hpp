/*
    Name:        irc/ctcp/dcc/command.hpp
    Purpose:     DCC commands.
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/02/03
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_CTCP_DCC_COMMAND_HPP
#define IRC_CTCP_DCC_COMMAND_HPP

namespace irc  {
namespace ctcp {
/**
    DCC namespace.
    DCC stands for something like "Direct Client Connection".
    CTCP DCC extended data messages are used to negotiate file transfers between
    clients and to negotiate chat connections over tcp connections between
    two clients, with no IRC server involved. Connections between clients
    involve protocols other than the usual IRC protocol.
*/
namespace dcc  {

/** DCC commands. */
enum class command
{
/** To carry a secure conversation. */
    chat,

/** To send a file to the recipient. */
    send
};

} // namespace dcc
} // namespace ctcp
} // namespace irc

#endif // IRC_CTCP_DCC_COMMAND_HPP
