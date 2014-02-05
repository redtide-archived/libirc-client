/*
    Name:        irc/impl/message_parser.hpp
    Purpose:     IRC message parser.
    Author:      Joseph Dobson
    Modified by: Andrea Zanellato
    Created:     2014/01/16
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_IMPL_PARSER_HPP
#define IRC_IMPL_PARSER_HPP

//SPEEDS UP SPIRIT COMPILE TIMES
#define BOOST_SPIRIT_NO_PREDEFINED_TERMINALS 1
#define BOOST_SPIRIT_USE_PHOENIX_V3          1

#include "irc/command.hpp"
#include "irc/impl/command.hpp"
#include "irc/message.hpp"
#include "irc/prefix.hpp"
#include "irc/types.hpp"

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/qi.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

#include <string>

BOOST_FUSION_ADAPT_STRUCT
(
    irc::prefix,
    (std::string, nickname)
    (std::string, username)
    (std::string, hostname)
)
BOOST_FUSION_ADAPT_STRUCT
(
    irc::message,
    (irc::prefix,              prefix)
    (irc::command,             command)
    (std::vector<std::string>, params)
)
namespace qi  = boost::spirit::qi;
namespace phx = boost::phoenix;
namespace irc {

template <typename Iterator>
struct message_parser : qi::grammar<Iterator, message(), qi::space_type>
{
    template<typename Val>
    using rule = qi::rule<Iterator, Val(), qi::space_type>;

    // Space sensitive
    template<typename Val>
    using rule_ss = qi::rule<Iterator, Val()>;

    message_parser() : message_parser::base_type(msg)
    {
        qi::attr_type attr;
        qi::char_type char_;
        qi::int_type  int_;
        qi::lit_type  lit;
        qi::_1_type   _1;
        qi::_val_type _val;
/*
    The presence of a prefix is indicated with a single leading ASCII
    colon character (':', 0x3b), which MUST be the first character of the
    message itself.
    There MUST be NO gap (whitespace) between the colon and the prefix.
    The prefix is used by servers to indicate the true origin of the message.
    If the prefix is missing from the message, it is assumed to have originated
    from the connection from which it was received from.
    Clients SHOULD NOT use a prefix when sending a message;
    if they use one, the only valid prefix
    is the registered nickname associated with the client.

    prefix = servername / ( nickname [ [ "!" user ] "@" host ] )
*/
        pfx %= lit(':') >> ((+~char_(" :@?!")    >> // nickname
                            '!' >> +~char_(" @") >> // username
                            '@' >> +~char_(' '))    // hostname
                        |
                           (attr(std::string{}) >>
                            attr(std::string{}) >>
                            +~char_(' ')));         // server hostname
/*
    The command MUST either be a valid IRC command
    or a three (3) digit number represented in ASCII text.
*/
        verbose_cmd.add
        ("NICK",   command::nick)   ("KICK",    command::kick)
        ("ERROR",  command::error)  ("MODE",    command::mode)
        ("QUIT",   command::quit)   ("TOPIC",   command::topic)
        ("PING",   command::ping)   ("PONG",    command::pong)
        ("NOTICE", command::notice) ("JOIN",    command::join)
        ("PART",   command::part)   ("PRIVMSG", command::privmsg)
        ("INVITE", command::invite);

        cmd        %= verbose_cmd | int_[_val = phx::bind( to_command, _1 )];
        last_param %= lit(':') >> *~char_("\r");
        mid_param  %= +~char_(" \r");
        msg        %= -pfx >> cmd >> +( last_param | mid_param );
    }

private:
    qi::symbols<char, command> verbose_cmd;
    rule<command>              cmd;
    rule_ss<prefix>            pfx;
    rule_ss<std::string>       last_param;
    rule_ss<std::string>       mid_param;
    rule<message>              msg;
};

} // namespace irc

#endif // IRC_IMPL_PARSER_HPP
