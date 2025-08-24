//
// Copyright (c) 2025 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/ws_io
//

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/websocket/stream.hpp>
#include "test_suite.hpp"

namespace boost {
namespace ws_io {
namespace test {

//------------------------------------------------

struct success_handler
{
    bool pass = false;

    void
    operator()(system::error_code ec, ...)
    {
        pass = BOOST_TEST(! ec.failed());
    }
};

//------------------------------------------------

template<class MF, class T, class... Args0>
struct bind_wrapper
{
    MF mf_;
    T this_;
    //std::tuple<Args0...> args0_;

    template<class... Args>
    void
    operator()(Args&&... args) const
    {
        (this_.*mf_)(std::forward<Args>(args)...);
    }
};

template<class MF, class Arg0, class... Argn>
bind_wrapper<MF, Arg0, Argn...>
bind_front(
    MF&& mf,
    Arg0* arg0,
    Argn&&...)
{
    return bind_wrapper<MF, Arg0, Argn...>{ mf, arg0 };
}

//------------------------------------------------

/** Connect two TCP sockets together.
*/
template<class Executor>
bool
connect(
    asio::basic_stream_socket<asio::ip::tcp, Executor>& s1,
    asio::basic_stream_socket<asio::ip::tcp, Executor>& s2)

{
    BOOST_ASSERT(s1.get_executor() == s2.get_executor());
    try
    {
        asio::basic_socket_acceptor<
            asio::ip::tcp, Executor> a(s1.get_executor());
        auto ep = asio::ip::tcp::endpoint(
            asio::ip::make_address_v4("127.0.0.1"), 0);
        a.open(ep.protocol());
        a.set_option(
            asio::socket_base::reuse_address(true));
        a.bind(ep);
        a.listen(0);
        ep = a.local_endpoint();
        a.async_accept(s2, success_handler());
        s1.async_connect(ep, success_handler());
        s1.get_executor().context().restart();
        s1.get_executor().context().run();
        s1.get_executor().context().restart();
        if(! BOOST_TEST_EQ(s1.remote_endpoint(), s2.local_endpoint()))
            return false;
        if(! BOOST_TEST_EQ(s2.remote_endpoint(), s1.local_endpoint()))
            return false;
    }
    catch(std::exception const&)
    {
        BOOST_TEST_FAIL();
        return false;
    }

    return true;
}

//------------------------------------------------

class session
{
public:
    using socket_type =
        asio::basic_stream_socket<
            asio::ip::tcp,
            asio::io_context::executor_type>;
    using executor_type = typename
        socket_type::executor_type;

private:
    beast::websocket::stream<socket_type> ws_;
    socket_type client_socket_;
    beast::flat_buffer buf_;

public:
    template<class Executor>
    explicit
    session(Executor const& ex)
        : ws_(ex)
        , client_socket_(ex)
    {
        connect(ws_.next_layer(), client_socket_);
        ws_.async_accept(
            [&](system::error_code ec)
            {
                on_accept(ec);
            });
    }

    socket_type
    release_client()
    {
        return std::move(client_socket_);
    }

    void
    on_accept(
        system::error_code ec)
    {
        if(! BOOST_TEST(! ec.failed()))
        {
            BOOST_ERROR(ec.message().data());
            return;
        }
        BOOST_TEST_PASS();
        do_read();
    }

    void
    do_read()
    {
        ws_.async_read(buf_,
            [&](system::error_code ec,
                std::size_t bytes_transferred)
            {
                on_read(ec, bytes_transferred);
            });
    }

    void
    on_read(
        system::error_code ec,
        std::size_t bytes_transferred)
    {
        (void)bytes_transferred;
        if(ec == beast::websocket::error::closed)
        {
            BOOST_TEST_PASS();
            return;
        }
        if(! BOOST_TEST(! ec.failed()))
        {
            BOOST_ERROR(ec.message().data());
            return;
        }
        BOOST_TEST_PASS();
        ws_.async_write(buf_.data(),
            [&](system::error_code ec,
                std::size_t bytes_transferred)
            {
                on_write(ec, bytes_transferred);
            });
    }

    void
    on_write(
        system::error_code ec,
        std::size_t bytes_transferred)
    {
        (void)bytes_transferred;
        if(ec == beast::websocket::error::closed)
        {
            BOOST_TEST_PASS();
            return;
        }
        if(! BOOST_TEST(! ec.failed()))
        {
            BOOST_ERROR(ec.message().data());
            return;
        }
        buf_.clear();
        do_read();
    }
};

//------------------------------------------------

} // test
} // ws_io
} // boost
