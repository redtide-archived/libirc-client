/*
    Name:        irc/impl/ctcp/parser.hpp
    Purpose:     IRC message parser.
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/01/21
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_IMPL_CTCP_PARSER_HPP
#define IRC_IMPL_CTCP_PARSER_HPP

//SPEEDS UP SPIRIT COMPILE TIMES
#define BOOST_SPIRIT_NO_PREDEFINED_TERMINALS 1
#define BOOST_SPIRIT_USE_PHOENIX_V3          1

#include "irc/ctcp/command.hpp"

#include <boost/fusion/include/vector.hpp>
#include <boost/spirit/home/qi.hpp>

#include <string>

namespace fsn = boost::fusion;
namespace irc {
namespace ctcp {

template<typename Iterator>
struct parser : qi::grammar<Iterator, fsn::vector<ctcp::command, std::string>(), qi::space_type>
{
    template<typename Val>
    using rule = qi::rule<Iterator, Val(), qi::space_type>;

    // Space sensitive
    template<typename Val>
    using rule_ss = qi::rule<Iterator, Val()>;

    parser() : parser::base_type(ctcp_msg)
    {
        qi::attr_type attr;
        qi::char_type char_;
        qi::lit_type  lit;

        ctcp_cmd.add
        ("ACTION",  ctcp::command::action)("CLIENTINFO", ctcp::command::clientinfo)
        ("DCC",     ctcp::command::dcc)   ("ERRMSG",     ctcp::command::errmsg)
        ("FINGER",  ctcp::command::finger)("PING",       ctcp::command::ping)
        ("SED",     ctcp::command::sed)   ("SOURCE",     ctcp::command::source)
        ("TIME",    ctcp::command::time)  ("USERINFO",   ctcp::command::userinfo)
        ("VERSION", ctcp::command::version);

        ctcp_args %= +~char_('\001');
        ctcp_msg  %= ( (lit('\001') >> ctcp_cmd) >> -ctcp_args )
                   | ( (attr(ctcp::command::none) >> attr(std::string{})) );
    }

private:
    rule_ss<std::string>                          ctcp_args;
    qi::symbols<char, ctcp::command>              ctcp_cmd;
    rule<fsn::vector<ctcp::command, std::string>> ctcp_msg;
};

} // namespace ctcp
} // namespace irc

#endif // IRC_IMPL_CTCP_PARSER_HPP
