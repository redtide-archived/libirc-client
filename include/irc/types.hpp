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
#include "irc/ctcp/command.hpp"
#include "irc/dcc/request.hpp"

#include <boost/signals2/signal.hpp>

namespace irc {

namespace signals = boost::signals2;

using sig_message = signals::signal<void(const message&)>;
using sig_void    = signals::signal<void()>;
using sig_ctcp    = signals::signal<void(const message&,
                                        irc::ctcp::command,
                                        const std::string&)>;
using sig_dcc_req = signals::signal<void(const irc::prefix&, const irc::dcc::request&)>;
using sig_dcc_msg = signals::signal<void(const irc::prefix&, const std::string&)>;
using sig_dcc_send= signals::signal<void(const irc::prefix&, size_t)>;

} // namespace irc

#endif // IRC_CLIENT_TYPES_HPP
