//
// Copyright (c) 2025 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/ws_io
//

#include <boost/asio/ip/tcp.hpp>
#include "test_suite.hpp"

namespace boost {
namespace ws_io {
namespace test {

//------------------------------------------------

using socket_type =
    asio::basic_stream_socket<
        asio::ip::tcp,
        asio::io_context::executor_type>;

class server
{
public:
    server();
    ~server();

    socket_type
    connect();

    void
    run();

private:
    struct impl;
    impl* impl_;
};

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

} // test
} // ws_io
} // boost
