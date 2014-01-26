/*
    Name:        irc/prefix.hpp
    Purpose:     IRC message prefix interface
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/01/24
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_PREFIX_HPP
#define IRC_PREFIX_HPP

#include <string>

namespace irc {
/**
    IRC message prefix.
*/
struct prefix
{
/** User nickname or empty() if this is a server. */
    std::string nickname;

/** User name or empty() if this is a server. */
    std::string username;
/**
    User or server hostname.
*/
    std::string hostname;
};

} // namespace irc

#endif // IRC_PREFIX_HPP
