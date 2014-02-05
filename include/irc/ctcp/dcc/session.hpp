/*
    Name:        irc/ctcp/dcc/session.hpp
    Purpose:     DCC session interface
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/02/03
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_CTCP_DCC_SESSION_HPP
#define IRC_CTCP_DCC_SESSION_HPP

#include "irc/ctcp/dcc/command.hpp"
#include "irc/error.hpp"
#include "irc/types.hpp"

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/system/error_code.hpp>

#include <memory>
#include <string>

namespace asio = boost::asio;
namespace ip   = boost::asio::ip;
namespace sys  = boost::system;
namespace ph   = std::placeholders;
namespace irc  {
namespace ctcp {
namespace dcc  {
/**
    @class session

    DCC session class.
*/
class session: public std::enable_shared_from_this< session >
             , public asio::coroutine
             , boost::noncopyable
{
public:
/** Shared DCC session pointer */
    typedef std::shared_ptr< session > ptr;
/**
    Static constructor.
    @param io_service Reference to the ASIO io_service controller.
    @return Shared pointer to a new DCC session object.
*/
    static ptr make_shared( asio::io_service &io_service );

/** Destructor. */
    ~session();

    void connect( const std::string &hostname,
                  const std::string &port,
                  ctcp::dcc::command dcctype,
                  const std::string &filename = std::string{},
                  long               filesize = 0 );

/** Disconnects the DCC connection. */
    void disconnect();

/** @return The DCC connection type. */
    irc::ctcp::dcc::command type() const;

private:
    explicit session( asio::io_service &io_service )
    :   m_io_service(io_service),
        m_socket(io_service),
        m_connected(false),
        m_lasterror(error_code::success)
    {
    }

    session() = delete;

    void loop       ( const sys::error_code &ec, size_t bytes );
    void handle_read( const sys::error_code &ec, size_t bytes );

    asio::io_service  &m_io_service;
    ip::tcp::socket    m_socket;
    asio::streambuf    m_buf_read,
                       m_buf_write;
    bool               m_connected;
    error_code         m_lasterror;

    ctcp::dcc::command m_type;      // chat or send
    std::string        m_filename;
    long               m_filesize;
};

} // namespace dcc
} // namespace ctcp
} // namespace irc

#ifdef IRC_CLIENT_HEADER_ONLY
    #include "irc/impl/ctcp/dcc/session.ipp"
    #include "irc/impl/ctcp/dcc/loop.ipp"
#endif

#endif // IRC_CTCP_DCC_SESSION_HPP
