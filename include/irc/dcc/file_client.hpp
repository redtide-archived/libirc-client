/*
    Name:        irc/dcc/file_client.hpp
    Purpose:     DCC chat client interface
    Author:      Andrea Zanellato
    Modified by: 
    Created:     2014/02/13
    Licence:     Boost Software License, Version 1.0
*/
#ifndef IRC_DCC_FILE_CLIENT_HPP
#define IRC_DCC_FILE_CLIENT_HPP

#include "irc/dcc/client.hpp"

#include <boost/filesystem.hpp>
#include <boost/noncopyable.hpp>

#include <memory>

namespace irc  {
namespace dcc  {

class file_client : public std::enable_shared_from_this< file_client >,
                    public irc::dcc::client,
                    public boost::asio::coroutine,
                    boost::noncopyable
{
public:
/** Shared DCC file client pointer */
    typedef std::shared_ptr< file_client > ptr;

    file_client::ptr make_shared(boost::asio::io_service &ios,
                                irc::prefix              details,
                                const std::string        &address,
                                const std::string        &port
                                const std::string        &fullpath)
{
    namespace fs  = boost::filesystem;
    namespace sys = boost::system;

    sys::error_code ec;
    fs::path fpath( fullpath );

    if( fs::is_regular_file(fs::status(fpath, ec)) )
    {
        file_client::ptr new_session
        (
            new file_client(ios, details, address, port, fpath)
        );
        return new_session;
    }
    return session::ptr(nullptr);
}
/**
    Signal fired when sending a file using DCC.
    @param func The function to call back.
    @return The connection object to disconnect from the signal.
*/
    template<typename Callback>
    signals::connection on_dcc_send( Callback&& func )
    { return m_on_dcc_send.connect( std::forward<Callback>(func) ); }

private:
    explicit session(boost::asio::io_service      &ios,
                    prefix                        details,
                    const std::string             &addr,
                    const std::string             &port,
                    const boost::filesystem::path &filepath)
    :   client( ios, details, addr, port ),
        m_filepath(filepath)
    {
        m_buf_read.prepare(512);
        m_buf_write.prepare(512);
    }

    boost::filesystem::path m_filepath;
    sig_dcc_send            m_on_dcc_send;
};

} // namespace dcc
} // namespace irc

#ifdef IRC_CLIENT_HEADER_ONLY
    #include "irc/impl/dcc/file_client.ipp"
    #include "irc/impl/dcc/loop.ipp"
#endif

#endif // IRC_DCC_FILE_CLIENT_HPP
