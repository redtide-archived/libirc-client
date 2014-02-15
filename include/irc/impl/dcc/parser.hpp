/*
    Name:        irc/impl/dcc/parser.hpp
    Purpose:     IRC DCC parser.
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/02/02
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_IMPL_DCC_PARSER_HPP
#define IRC_IMPL_DCC_PARSER_HPP

//SPEEDS UP SPIRIT COMPILE TIMES
#define BOOST_SPIRIT_NO_PREDEFINED_TERMINALS 1
#define BOOST_SPIRIT_USE_PHOENIX_V3          1

#include "irc/dcc/command.hpp"
#include "irc/dcc/request.hpp"

#include <boost/fusion/include/vector.hpp>
#include <boost/spirit/home/qi.hpp>

#include <string>

BOOST_FUSION_ADAPT_STRUCT
(
    irc::dcc::request,
    (irc::dcc::command, type)
    (std::string, argument)
    (std::string, address)
    (std::string, port)
    (long, size)
)

namespace irc  {
namespace dcc  {

namespace fsn = boost::fusion;
namespace qi  = boost::spirit::qi;

template<typename Iterator>
struct parser : qi::grammar<Iterator, request(), qi::space_type>
{
    template<typename Val>
    using rule = qi::rule<Iterator, Val(), qi::space_type>;

    // Space sensitive
    template<typename Val>
    using rule_ss = qi::rule<Iterator, Val()>;

    parser() : parser::base_type(req)
    {
        qi::char_type   char_;
        qi::long_type   long_;

        type.add ("CHAT", command::chat) ("SEND", command::send);
        argument %= +~char_(' ');
        address  %= +~char_(' ');
        port     %= +~char_(' ');
        req      %= type >> argument >> address >> port >> -long_;
    }

private:
    qi::symbols<char, command> type;
    rule_ss<std::string>       argument;
    rule_ss<std::string>       address;
    rule_ss<std::string>       port;
    rule<request>              req;
};

} // namespace dcc
} // namespace irc

#endif // IRC_IMPL_DCC_PARSER_HPP
