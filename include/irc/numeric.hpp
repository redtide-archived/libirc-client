/*
    Name:,       irc/numeric.hpp
    Purpose:,    IRC Events: numeric reply and error codes
    Author:,     Andrea Zanellato
    Modified by: 
    Created:,    2011/10/05
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_NUMERIC_HPP
#define IRC_NUMERIC_HPP

// E = EFnet hybrid-6rc4
// I = IRCnet 2.10.3
// i = IRCnet, reserved according to Undernet (sic) numerics headers
// U = Undernet u2.10.07
// D = DALnet bahamut 1.4.8

namespace irc {

enum class reply_code
{
    RPL_WELCOME           = 1,    // IEUD
    RPL_YOURHOST,                 // IEUD
    RPL_CREATED,                  // IEUD
    RPL_MYINFO,                   // IEUD
    RPL_BOUNCE            = 5,    // I
    RPL_MAP               = 5,    // U
    RPL_PROTOCTL          = 5,    // D
    RPL_MAPMORE,                  // U
    RPL_MAPEND,                   // U
    RPL_SNOMASK,                  // U
    RPL_STATMEMTOT,               // U
    RPL_STATMEM,                  // U
    RPL_YOURCOOKIE        = 14,   // i
    RPL_TRACELINK         = 200,  // IEUD
    RPL_TRACECONNECTING,          // IEUD
    RPL_TRACEHANDSHAKE,           // IEUD
    RPL_TRACEUNKNOWN,             // IEUD
    RPL_TRACEOPERATOR,            // IEUD
    RPL_TRACEUSER,                // IEUD
    RPL_TRACESERVER,              // IEUD
    RPL_TRACESERVICE,             // IEU
    RPL_TRACENEWTYPE,             // IEUD
    RPL_TRACECLASS,               // IEUD
    RPL_TRACERECONNECT,           // I
    RPL_STATSLINKINFO,            // IEUD
    RPL_STATSCOMMANDS,            // IEUD
    RPL_STATSCLINE,               // IEUD
    RPL_STATSNLINE,               // IEUD
    RPL_STATSILINE,               // IEUD
    RPL_STATSKLINE,               // IEUD
    RPL_STATSQLINE        = 217,  // IED
    RPL_STATSPLINE        = 217,  // U
    RPL_STATSYLINE,               // IEUD
    RPL_ENDOFSTATS,               // IEUD
    RPL_UMODEIS,                  // IEUD
    RPL_STATSBLINE,               // ED
    RPL_STATSELINE,               // ED
    RPL_STATSFLINE,               // ED
    RPL_STATSDLINE        = 225,  // E
    RPL_STATSZLINE        = 225,  // D
    RPL_STATSCOUNT,               // D
    RPL_STATSGLINE,               // D
    RPL_SERVICEINFO       = 231,  // IU
    RPL_ENDOFSERVICES,            // IU
    RPL_SERVICE,                  // IU
    RPL_SERVLIST,                 // IEUD
    RPL_SERVLISTEND,              // IEUD
    RPL_STATSIAUTH        = 239,  // I
    RPL_STATSVLINE,               // I
    RPL_STATSLLINE,               // IEUD
    RPL_STATSUPTIME,              // IEUD
    RPL_STATSOLINE,               // IEUD
    RPL_STATSHLINE,               // IEUD
    RPL_STATSSLINE,               // IEuD
    RPL_STATSPING         = 246,  // I
    RPL_STATSTLINE        = 246,  // U
    RPL_STATSULINE        = 246,  // D
    RPL_STATSBLINE_I      = 247,  // I
    RPL_STATSGLINE_TODO   = 247,  // U (REDEFINES see above)
    RPL_STATSXLINE        = 247,  // E
    RPL_STATSDEFINE       = 248,  // I
    RPL_STATSULINE_TODO   = 248,  // EU (REDEFINES see above)
    RPL_STATSDEBUG,               // IEUD
    RPL_STATSDLINE_TODO_1 = 250,  // I (REDEFINES see above)
    RPL_STATSCONN         = 250,  // EUD
    RPL_LUSERCLIENT,              // IEUD
    RPL_LUSEROP,                  // IEUD
    RPL_LUSERUNKNOWN,             // IEUD
    RPL_LUSERCHANNELS,            // IEUD
    RPL_LUSERME,                  // IEUD
    RPL_ADMINME,                  // IEUD
    RPL_ADMINLOC1,                // IEUD
    RPL_ADMINLOC2,                // IEUD
    RPL_ADMINEMAIL,               // IEUD
    RPL_TRACELOG          = 261,  // IEUD
    RPL_TRACEEND          = 262,  // I
    RPL_ENDOFTRACE        = 262,  // ED
    RPL_TRACEPING         = 262,  // U
    RPL_TRYAGAIN          = 263,  // I
    RPL_LOAD2HI           = 263,  // ED
    RPL_LOCALUSERS        = 265,  // ED
    RPL_GLOBALUSERS,              // ED
    RPL_SILELIST          = 271,  // UD
    RPL_ENDOFSILELIST,            // UD
    RPL_STATSDELTA        = 274,  // i
    RPL_STATSDLINE_TODO_2,        // UD (REDEFINES see above)
    RPL_GLIST             = 280,  // U
    RPL_ENDOFGLIST,               // U
    RPL_HELPHDR           = 290,  // D
    RPL_HELPOP,                   // D
    RPL_HELPTLR,                  // D
    RPL_HELPHLP,                  // D
    RPL_HELPFWD,                  // D
    RPL_HELPIGN,                  // D
    RPL_NONE              = 300,  // IEUD
    RPL_AWAY,                     // IEUD
    RPL_USERHOST,                 // IEUD
    RPL_ISON,                     // IEUD
    RPL_TEXT,                     // IEUD
    RPL_UNAWAY,                   // IEUD
    RPL_NOWAWAY,                  // IEUD
    RPL_USERIP            = 307,  // U
    RPL_WHOISREGNICK      = 307,  // D
    RPL_WHOISADMIN,               // D
    RPL_WHOISSADMIN,              // D
    RPL_WHOISSVCMSG,              // D
    RPL_WHOISUSER,                // IEUD
    RPL_WHOISSERVER,              // IEUD
    RPL_WHOISOPERATOR,            // IEUD
    RPL_WHOWASUSER,               // IEUD
    RPL_ENDOFWHO,                 // IEUD
    RPL_WHOISCHANOP,              // IED
    RPL_WHOISIDLE,                // IEUD
    RPL_ENDOFWHOIS,               // IEUD
    RPL_WHOISCHANNELS,            // IEUD
    RPL_LISTSTART         = 321,  // IEUD
    RPL_LIST,                     // IEUD
    RPL_LISTEND,                  // IEUD
    RPL_CHANNELMODEIS,            // IEUD
    RPL_UNIQOPIS,                 // I
    RPL_NOCHANPASS,               // i
    RPL_CHPASSUNKNOWN,            // i
    RPL_CREATIONTIME      = 329,  // EUD
    RPL_NOTOPIC           = 331,  // IEUD
    RPL_TOPIC,                    // IEUD
    RPL_TOPICWHOTIME,             // EUD
    RPL_LISTUSAGE         = 334,  // U
    RPL_COMMANDSYNTAX     = 334,  // D
    RPL_CHANPASSOK        = 338,  // i
    RPL_BADCHANPASS,              // i
    RPL_INVITING          = 341,  // IEUD
    RPL_SUMMONING,                // IED
    RPL_INVITELIST        = 346,  // I
    RPL_ENDOFINVITELIST,          // I
    RPL_EXCEPTLIST,               // IE
    RPL_ENDOFEXCEPTLIST,          // IE
    RPL_VERSION           = 351,  // IEUD
    RPL_WHOREPLY,                 // IEUD
    RPL_NAMREPLY,                 // IEUD
    RPL_WHOSPCRPL,                // U
    RPL_KILLDONE          = 361,  // IEUD
    RPL_CLOSING,                  // IEUD
    RPL_CLOSEEND,                 // IEUD
    RPL_LINKS,                    // IEUD
    RPL_ENDOFLINKS,               // IEUD
    RPL_ENDOFNAMES,               // IEUD
    RPL_BANLIST,                  // IEUD
    RPL_ENDOFBANLIST,             // IEUD
    RPL_ENDOFWHOWAS,              // IEUD
    RPL_INFO              = 371,  // IEUD
    RPL_MOTD,                     // IEUD
    RPL_INFOSTART,                // IEUD
    RPL_ENDOFINFO,                // IEUD
    RPL_MOTDSTART,                // IEUD
    RPL_ENDOFMOTD,                // IEUD
    RPL_YOUREOPER         = 381,  // IEUD
    RPL_REHASHING,                // IEUD
    RPL_YOURESERVICE,             // ID
    RPL_MYPORTIS,                 // IEUD
    RPL_NOTOPERANYMORE,           // IEUD
    RPL_TIME              = 391,  // IEUD
    RPL_USERSSTART,               // IED
    RPL_USERS,                    // IED
    RPL_ENDOFUSERS,               // IED
    RPL_NOUSERS,                  // IED
    RPL_LOGON             = 600,  // D
    RPL_LOGOFF,                   // D
    RPL_WATCHOFF,                 // D
    RPL_WATCHSTAT,                // D
    RPL_NOWON,                    // D
    RPL_NOWOFF,                   // D
    RPL_WATCHLIST,                // D
    RPL_ENDOFWATCHLIST,           // D
    RPL_DCCSTATUS         = 617,  // D
    RPL_DCCLIST,                  // D
    RPL_ENDOFDCCLIST,             // D
    RPL_DCCINFO,                  // D
    ERR_NOSUCHNICK        = 401,  // IEUD
    ERR_NOSUCHSERVER,             // IEUD
    ERR_NOSUCHCHANNEL,            // IEUD
    ERR_CANNOTSENDTOCHAN,         // IEUD
    ERR_TOOMANYCHANNELS,          // IEUD
    ERR_WASNOSUCHNICK,            // IEUD
    ERR_TOOMANYTARGETS,           // IEUD
    ERR_NOSUCHSERVICE     = 408,  // I
    ERR_NOCOLORSONCHAN    = 408,  // D
    ERR_NOORIGIN,                 // IEUD
    ERR_NORECIPIENT       = 411,  // IEUD
    ERR_NOTEXTTOSEND,             // IEUD
    ERR_NOTOPLEVEL,               // IEUD
    ERR_WILDTOPLEVEL,             // IEUD
    ERR_BADMASK,                  // I
    ERR_TOOMANYMATCHES    = 416,  // I
    ERR_QUERYTOOLONG      = 416,  // U
    ERR_UNKNOWNCOMMAND    = 421,  // IEUD
    ERR_NOMOTD,                   // IEUD
    ERR_NOADMININFO,              // IEUD
    ERR_FILEERROR,                // IED
    ERR_TOOMANYAWAY       = 429,  // D
    ERR_NONICKNAMEGIVEN   = 431,  // IEUD
    ERR_ERRONEUSNICKNAME,         // IEUD
    ERR_NICKNAMEINUSE,            // IEUD
    ERR_SERVICENAMEINUSE,         // ID
    ERR_SERVICECONFUSED   = 435,  // I
    ERR_BANONCHAN         = 435,  // D
    ERR_NICKCOLLISION,            // IEUD
    ERR_UNAVAILRESOURCE   = 437,  // IE
    ERR_BANNICKCHANGE     = 437,  // UD
    ERR_DEAD              = 438,  // I
    ERR_NICKTOOFAST       = 438,  // U
    ERR_NCHANGETOOFAST    = 438,  // D
    ERR_TARGETTOOFAST,            // UD
    ERR_SERVICESDOWN,             // D
    ERR_USERNOTINCHANNEL,         // IEUD
    ERR_NOTONCHANNEL,             // IEUD
    ERR_USERONCHANNEL,            // IEUD
    ERR_NOLOGIN,                  // IED
    ERR_SUMMONDISABLED,           // IED
    ERR_USERSDISABLED,            // IED
    ERR_NOTREGISTERED     = 451,  // IEUD
    ERR_IDCOLLISION,              // i
    ERR_NICKLOST,                 // i
    ERR_HOSTILENAME       = 455,  // D
    ERR_NEEDMOREPARAMS    = 461,  // IEUD
    ERR_ALREADYREGISTRED,         // IEUD
    ERR_NOPERMFORHOST,            // IEUD
    ERR_PASSWDMISMATCH,           // IEUD
    ERR_YOUREBANNEDCREEP,         // IEUD
    ERR_YOUWILLBEBANNED,          // IEUD
    ERR_KEYSET,                   // IEUD
    ERR_INVALIDUSERNAME   = 468,  // U
    ERR_ONLYSERVERSCANCHANGE = 468, // D
    ERR_CHANNELISFULL     = 471,  // IEUD
    ERR_UNKNOWNMODE,              // IEUD
    ERR_INVITEONLYCHAN,           // IEUD
    ERR_BANNEDFROMCHAN,           // IEUD
    ERR_BADCHANNELKEY,            // IEUD
    ERR_BADCHANMASK,              // IEUD
    ERR_MODELESS          = 477,  // E
    ERR_NOCHANMODES       = 477,  // I
    ERR_NEEDREGGEDNICK    = 477,  // D
    ERR_BANLISTFULL,              // IEUD
    ERR_BADCHANNAME,              // ED
    ERR_NOPRIVILEGES      = 481,  // IEUD
    ERR_CHANOPRIVSNEEDED,         // IEUD
    ERR_CANTKILLSERVER,           // EUD
    ERR_DESYNC            = 484,  // ED
    ERR_ISCHANSERVICE     = 484,  // U
    ERR_UNIQOPPRIVSNEEDED,        // I
    ERR_CHANTOORECENT     = 487,  // i
    ERR_TSLESSCHAN,               // i
    ERR_VOICENEEDED,              // U
    ERR_NOOPERHOST        = 491,  // IEUD
    ERR_NOSERVICEHOST,            // I
    ERR_UMODEUNKNOWNFLAG  = 501,  // IEUD
    ERR_USERSDONTMATCH,           // IEUD
    ERR_GHOSTEDCLIENT,            // ED
    ERR_LAST_ERR_MSG,             // ED
    ERR_SILELISTFULL      = 511,  // UD
    ERR_NOSUCHGLINE       = 512,  // U
    ERR_TOOMANYWATCH      = 512,  // D
    ERR_BADPING,                  // U
    ERR_TOOMANYDCC,               // D
    ERR_LISTSYNTAX        = 521,  // D
    ERR_WHOSYNTAX,                // D
    ERR_WHOLIMEXCEED,             // D
    ERR_NUMERIC_ERR       = 999   // D
};

} // namespace irc

#endif //IRC_NUMERIC_HPP

