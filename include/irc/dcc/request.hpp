/*
    Name:        irc/dcc/request.hpp
    Purpose:     DCC request message
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/01/08
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_DCC_REQUEST_HPP
#define IRC_DCC_REQUEST_HPP

#include "irc/dcc/command.hpp"

namespace irc  {
namespace dcc  {
/**
    
*/
struct request
{
/** . */
    irc::dcc::command type;

/** . */
    std::string argument;

/** . */
    std::string address;

/** . */
    std::string port;

/** . */
    long size;
};

} // namespace dcc
} // namespace irc

#endif // IRC_DCC_REQUEST_HPP
