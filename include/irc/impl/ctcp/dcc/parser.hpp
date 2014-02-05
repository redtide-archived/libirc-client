/*
    Name:        irc/impl/ctcp/dcc/parser.hpp
    Purpose:     IRC DCC parser.
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/02/02
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_IMPL_CTCP_DCC_PARSER_HPP
#define IRC_IMPL_CTCP_DCC_PARSER_HPP

//SPEEDS UP SPIRIT COMPILE TIMES
#define BOOST_SPIRIT_NO_PREDEFINED_TERMINALS 1
#define BOOST_SPIRIT_USE_PHOENIX_V3          1

#include "irc/ctcp/dcc/command.hpp"
#include <boost/fusion/include/vector.hpp>
#include <boost/spirit/home/qi.hpp>
#include <string>

namespace qi  = boost::spirit::qi;
namespace fsn = boost::fusion;
namespace irc {
namespace ctcp {
namespace dcc {

using message = fsn::vector<command, std::string, std::string, std::string, long>;

template<typename Iterator>
struct parser : qi::grammar<Iterator, message(), qi::space_type>
{
    template<typename Val>
    using rule = qi::rule<Iterator, Val(), qi::space_type>;

    // Space sensitive
    template<typename Val>
    using rule_ss = qi::rule<Iterator, Val()>;

    parser() : parser::base_type(msg)
    {
        qi::char_type   char_;
        qi::long_type   long_;

        type.add ("CHAT", command::chat) ("SEND", command::send);
        argument %= +~char_(' ');
        address  %= +~char_(' ');
        port     %= +~char_(' ');
        msg      %= type >> argument >> address >> port >> -long_;
    }

private:
    qi::symbols<char, command> type;
    rule_ss<std::string>       argument;
    rule_ss<std::string>       address;
    rule_ss<std::string>       port;
    rule<message>              msg;
};

} // namespace dcc
} // namespace ctcp
} // namespace irc

#endif // IRC_IMPL_CTCP_DCC_PARSER_HPP
