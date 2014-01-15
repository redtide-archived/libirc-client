/*
    Name:        irc/error.hpp
    Purpose:     libirc error codes
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/01/11
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_ERROR_HPP
#define IRC_ERROR_HPP

namespace irc {

enum class error_code   /** Error codes. */
{
    success         = 0,/**< Success. */
    invalid_request = 1 /**< Invalid request. */
};

} // namespace irc

#endif // IRC_ERROR_HPP
