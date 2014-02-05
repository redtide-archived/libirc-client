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

#include "irc/ctcp/command.hpp"
#include "irc/ctcp/dcc/session.hpp"
#include "irc/message.hpp"

#include <boost/asio.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/system/error_code.hpp>

namespace signals = boost::signals2;
namespace irc {

using sig_message= signals::signal<void(const message&)>;
using sig_void   = signals::signal<void()>;
using sig_ctcp   = signals::signal<void(const message&,
                                        ctcp::command,
                                        const std::string&)>;
using sig_dcc_req= signals::signal<void(const std::string&, const std::string&,
                                        ctcp::dcc::command, const std::string&,
                                        long)>;

} // namespace irc

#endif // IRC_CLIENT_TYPES_HPP
