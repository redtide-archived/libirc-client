/*
    Name:        irc/message.hpp
    Purpose:     IRC message interface
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/01/08
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_MESSAGE_HPP
#define IRC_MESSAGE_HPP

#include "irc/command.hpp"
#include "irc/prefix.hpp"

#include <vector>

namespace irc {
/**
    Servers and clients send each other messages,
    which may or may not generate a reply.
*/
struct message
{
/** Message prefix (optional). */
    irc::prefix  prefix;

/** Message command. */
    irc::command command;
/**
    Message command parameters.
    RFC set this to a maximum of 15.
*/
    std::vector<std::string> params;
};

} // namespace irc

#endif // IRC_MESSAGE_HPP
