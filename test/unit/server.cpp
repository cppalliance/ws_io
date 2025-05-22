//
// Copyright (c) 2016-2025 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#if 0

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

// Report a failure
void
fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// Echoes back all received WebSocket messages
class session : public std::enable_shared_from_this<session>
{
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;

public:
    // Take ownership of the socket
    explicit
    session(tcp::socket&& socket)
        : ws_(std::move(socket))
    {
    }

    // Get on the correct executor
    void
    run()
    {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        asio::dispatch(ws_.get_executor(),
            beast::bind_front_handler(
                &session::on_run,
                shared_from_this()));
    }

    // Start the asynchronous operation
    void
    on_run()
    {
        // Set suggested timeout settings for the websocket
        ws_.set_option(
            websocket::stream_base::timeout::suggested(
                beast::role_type::server));

        // Set a decorator to change the Server of the handshake
        ws_.set_option(websocket::stream_base::decorator(
            [](websocket::response_type& res)
            {
                res.set(http::field::server,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-server-async");
            }));
        // Accept the websocket handshake
        ws_.async_accept(
            beast::bind_front_handler(
                &session::on_accept,
                shared_from_this()));
    }

    void
    on_accept(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "accept");

        // Read a message
        do_read();
    }

    void
    do_read()
    {
        // Read a message into our buffer
        ws_.async_read(
            buffer_,
            beast::bind_front_handler(
                &session::on_read,
                shared_from_this()));
    }

    void
    on_read(
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // This indicates that the session was closed
        if(ec == websocket::error::closed)
            return;

        if(ec)
            return fail(ec, "read");

        // Echo the message
        ws_.text(ws_.got_text());
        ws_.async_write(
            buffer_.data(),
            beast::bind_front_handler(
                &session::on_write,
                shared_from_this()));
    }

    void
    on_write(
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");

        // Clear the buffer
        buffer_.consume(buffer_.size());

        // Do another read
        do_read();
    }
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
    asio::io_context& ioc_;
    tcp::acceptor acceptor_;

public:
    listener(
        asio::io_context& ioc,
        tcp::endpoint endpoint)
        : ioc_(ioc)
        , acceptor_(ioc)
    {
        beast::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if(ec)
        {
            fail(ec, "open");
            return;
        }

        // Allow address reuse
        acceptor_.set_option(asio::socket_base::reuse_address(true), ec);
        if(ec)
        {
            fail(ec, "set_option");
            return;
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if(ec)
        {
            fail(ec, "bind");
            return;
        }

        // Start listening for connections
        acceptor_.listen(
            asio::socket_base::max_listen_connections, ec);
        if(ec)
        {
            fail(ec, "listen");
            return;
        }
    }

    // Start accepting incoming connections
    void
    run()
    {
        do_accept();
    }

private:
    void
    do_accept()
    {
        // The new connection gets its own strand
        acceptor_.async_accept(
            asio::make_strand(ioc_),
            beast::bind_front_handler(
                &listener::on_accept,
                shared_from_this()));
    }

    void
    on_accept(beast::error_code ec, tcp::socket socket)
    {
        if(ec)
        {
            fail(ec, "accept");
        }
        else
        {
            // Create the session and run it
            std::make_shared<session>(std::move(socket))->run();
        }

        // Accept another connection
        do_accept();
    }
};

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    // Check command line arguments.
    if (argc != 4)
    {
        std::cerr <<
            "Usage: websocket-server-async <address> <port> <threads>\n" <<
            "Example:\n" <<
            "    websocket-server-async 0.0.0.0 8080 1\n";
        return EXIT_FAILURE;
    }
    auto const address = asio::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    auto const threads = std::max<int>(1, std::atoi(argv[3]));

    // The io_context is required for all I/O
    asio::io_context ioc{threads};

    // Create and launch a listening port
    std::make_shared<listener>(ioc, tcp::endpoint{address, port})->run();

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
        v.emplace_back(
        [&ioc]
        {
            ioc.run();
        });
    ioc.run();

    return EXIT_SUCCESS;
}

#endif

#include "test/unit/server.hpp"
#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <boost/assert.hpp>
#include <boost/smart_ptr/enable_shared_from.hpp>
#include <memory>
#include <thread>
#include <utility>

#include "test_suite.hpp"

namespace boost {
namespace ws_io {
namespace test {

//------------------------------------------------

template<class MF, class T, class... Args0>
struct bind_wrapper
{
    MF mf_;
    boost::shared_ptr<T> self_;
    //std::tuple<Args0...> args0_;

    template<class... Args>
    void
    operator()(Args&&... args) const
    {
        (self_.get()->*mf_)(std::forward<Args>(args)...);
    }
};

template<class MF, class Arg0, class... Argn>
bind_wrapper<MF, Arg0, Argn...>
bind_front(
    MF&& mf,
    Arg0* arg0,
    Argn&&...)
{
    return bind_wrapper<MF, Arg0, Argn...>{ mf, shared_from(arg0) };
}

//------------------------------------------------

class session
    : public boost::enable_shared_from
{
    beast::websocket::stream<socket_type> ws_;

public:
    session(socket_type&& sock)
        : ws_(std::move(sock))
    {
    }

    void
    run()
    {
        ws_.async_accept(
            bind_front(&session::on_accept, this));
    }

    void
    on_accept(
        system::error_code ec)
    {
        if(! BOOST_TEST(! ec.failed()))
            return;
        BOOST_TEST_PASS();

    }
};

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

struct server::impl
{
    asio::io_context ioc_;

    impl()
    {
    }

    ~impl()
    {
    }

    socket_type
    connect()
    {
        socket_type s0(ioc_.get_executor());
        socket_type s1(ioc_.get_executor());
        test::connect(s0, s1);
        auto s = boost::make_shared<session>(std::move(s0));
        s->run();
        return s1;
    }

    void
    run()
    {
        ioc_.restart();
        ioc_.run();
    }
};

server::
server()
    : impl_(new impl)
{
}

server::
~server()
{
    delete impl_;
}

socket_type
server::
connect()
{
    return impl_->connect();
}

void
server::
run()
{
    return impl_->run();
}

} // test
} // ws_io
} // boost
