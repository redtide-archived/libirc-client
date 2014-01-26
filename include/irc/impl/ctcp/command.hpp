/*
    Name:        irc/impl/ctcp/command.ipp
    Purpose:     CTCP request and reply command parser.
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/01/21
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_IMPL_CTCP_COMMAND_IPP
#define IRC_IMPL_CTCP_COMMAND_IPP

namespace irc {
namespace ctcp {

command to_command( raw_ctcp_command_t value )
{
    switch(value)
    {
    default:
        return command::none;
    case static_cast<raw_command_t>(command::action):
    case static_cast<raw_command_t>(command::clientinfo):
    case static_cast<raw_command_t>(command::dcc):
    case static_cast<raw_command_t>(command::errmsg):
    case static_cast<raw_command_t>(command::finger):
    case static_cast<raw_command_t>(command::ping):
    case static_cast<raw_command_t>(command::sed):
    case static_cast<raw_command_t>(command::source):
    case static_cast<raw_command_t>(command::time):
    case static_cast<raw_command_t>(command::userinfo):
    case static_cast<raw_command_t>(command::version):
        return static_cast<command>(value);
    }
}

} // namespace ctcp
} // namespace irc

#endif // IRC_IMPL_CTCP_COMMAND_IPP
