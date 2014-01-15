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

#include <vector>

namespace irc {

class message
{
public:
    typedef std::vector<std::string> params_type;

    message(const std::string &sender,
            const std::string &str_cmd,
            reply_code         int_cmd,
            params_type        params)
    :   m_sender(sender),
        m_command(str_cmd),
        m_rpl_code(int_cmd),
        m_params(params)
    {}

    std::string sender()  const { return m_sender; }
    std::string command() const { return m_command; }
    reply_code  code()    const { return m_code; }
    params_type params()  const { return m_params; }

private:
    std::string m_sender,
                m_command;
    reply_code  m_code;
    params_type m_params;
};

} // namespace irc

#endif // IRC_MESSAGE_HPP
