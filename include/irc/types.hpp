/*
    Name:        irc/types.hpp
    Purpose:     IRC client types.
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/01/17
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_CLIENT_TYPES_HPP
#define IRC_CLIENT_TYPES_HPP

#include "irc/message.hpp"

#include <boost/signals2/signal.hpp>

namespace irc {

class  message;

namespace signals = boost::signals2;
using sig_message = signals::signal<void(const message&)>;
using sig_void    = signals::signal<void()>;
using sig_ctcp    = signals::signal<void(const std::string&, const std::string&)>;

} // namespace irc

#endif // IRC_CLIENT_TYPES_HPP
