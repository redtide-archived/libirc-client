/*
    Name:        irc/dcc/session.hpp
    Purpose:     DCC session interface
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/02/03
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_DCC_SESSION_HPP
#define IRC_DCC_SESSION_HPP

#include "irc/types.hpp"
#include "irc/error.hpp"
#include "irc/prefix.hpp"
#include "irc/dcc/command.hpp"

#include <boost/asio.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/system/error_code.hpp>

#include <string>

namespace irc  {
namespace dcc  {

/**  DCC session class. */
class session
{
public:
/**
    Static constructor.
    @param ios      Reference to the ASIO io_service controller.
    @param host     The recipient's hostmask.
    @param type     The DCC type.
    @param address  Our address if offering the DCC, the recipient one otherwise.
    @param port     Our port if offering the DCC, the recipient one otherwise.
    @param serve    @true if we are offering the DCC, @false otherwise.
    @return Shared pointer to a new DCC session object.

    static ptr make_shared( boost::asio::io_service &ios,
                            irc::dcc::command type,
                            irc::prefix             host,
                            const std::string       &address,
                            const std::string       &port,
                            bool                    serve = false );
*/
/**
    Static constructor for DCC send.
    @param ios      Reference to the ASIO io_service controller.
    @param host     The recipient's hostmask.
    @param address  Our address if offering the DCC, the recipient one otherwise.
    @param port     Our port if offering the DCC, the recipient one otherwise.
    @param path     The full path to the file to send.
    @param serve    @true if we are offering the DCC, @false otherwise.
    @return Shared pointer to a new DCC session object.

    static ptr make_shared( boost::asio::io_service &ios,
                            irc::prefix             host,
                            const std::string       &address,
                            const std::string       &port,
                            const std::string       &fullpath,
                            bool                    serve = false );
*/
/** Destructor. */
    virtual ~session() {}

/** Connects the DCC session. */
    void connect() { do_connect(); }

/** Disconnects the DCC session. */
    void disconnect() { do_disconnect(); }

/** @return The own or recipient DCC address as string. */
    std::string address() const { return do_address(); }

/** @return The port number as string. */
    std::string port() const { return do_port(); }

/** @return The DCC connection type. */
    irc::dcc::command type() const { return do_type(); }

    bool active() const { return do_active(); }

//  irc::prefix hostmask() const { return m_prefix; }
/**
    Signal fired when the connection was enstablished.
    @param func The function to call back.
    @return The connection object to disconnect from the signal.
*/
    template<typename Callback>
    signals::connection on_connected( Callback&& func )
    { return m_on_connected.connect( std::forward<Callback>(func) ); }
/**
    Signal fired when closing the session.
    @param func The function to call back.
    @return The connection object to disconnect from the signal.
*/
    template<typename Callback>
    signals::connection on_disconnected( Callback&& func )
    { return m_on_disconnected.connect( std::forward<Callback>(func) ); }

protected:
    session(boost::asio::io_service &ios,
            irc::prefix             details,
            const std::string       &addr,
            const std::string       &port)
    :   m_ios(ios),
        m_socket(ios),
        m_address(addr),
        m_port(port),
        m_connected(false),
        m_prefix(details),
        m_lasterror(error_code::success)
    {
    }

private:
    session() = delete;

    virtual command do_type()   const = 0;
    virtual bool    do_active() const = 0;

    virtual std::string do_address() const { return m_address; }
    virtual std::string do_port()    const { return m_port; }

    virtual void do_connect() = 0;
    virtual void do_disconnect()
    {
        if( !m_connected )
        {
            m_lasterror = error_code::invalid_request;
            return;
        }

        m_connected = false;
        m_on_disconnected();

        m_socket.close();
    }

protected:
    virtual void do_read()  = 0;
    virtual void do_write() = 0;

    boost::asio::io_service     &m_ios;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::streambuf       m_buf_read,
                                 m_buf_write;
    std::string                  m_address,
                                 m_port;
    bool                         m_connected;
    irc::prefix                  m_prefix;
    error_code                   m_lasterror;
    sig_void                     m_on_connected,
                                 m_on_disconnected;
};

} // namespace dcc
} // namespace irc

#endif // IRC_DCC_SESSION_HPP
