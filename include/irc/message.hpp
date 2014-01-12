/*
    Name:,       irc/message.hpp
    Purpose:,    IRC message interface
    Author:,     Andrea Zanellato
    Modified by: 
    Created:,    2014/01/08
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_MESSAGE_HPP
#define IRC_MESSAGE_HPP

#include <vector>

namespace irc {

class message
{
public:
    typedef std::vector<std::string> params_type;

    message(const std::string &prefix,
            const std::string &str_cmd,
            int int_cmd,
            params_type params)
    :   m_prefix(prefix),
        m_str_cmd(str_cmd),
        m_int_cmd(int_cmd),
        m_params(params)
    {}

    std::string prefix()     const { return m_prefix; }
    std::string command()    const { return m_str_cmd; }
    reply_code  command_id() const { return m_int_cmd; }
    params_type params()     const { return m_params; }

private:
    std::string m_prefix,
                m_str_cmd;
    reply_code  m_int_cmd;
    params_type m_params;
};

} // namespace irc

#endif // IRC_MESSAGE_HPP
