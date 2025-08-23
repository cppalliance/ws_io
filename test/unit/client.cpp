//
// Copyright (c) 2025 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/ws_io
//

#include <boost/ws_io/client.hpp>
#include <boost/rts/context.hpp>
#include <boost/http_proto/parser.hpp>
#include "test/unit/server.hpp"
#include "test_suite.hpp"

namespace boost {
namespace ws_io {

struct client_test
{
    void
    run()
    {
        asio::io_context ioc;
        rts::context ctx;
        http_proto::parser::config_base cfg;
        http_proto::install_parser_service(ctx, cfg);

        test::session srv(ioc.get_executor());
        client<test::session::socket_type> cs(
            srv.release_client(), ctx);
        cs.async_handshake(
            "localhost",
            "/",
            [](http_proto::request&)
            {
            },
            [](system::error_code, http_proto::response_view)
            {
            });
        ioc.run();
    }
};

TEST_SUITE(
    client_test,
    "boost.ws_io.client");

} // ws_io
} // boost

/*

 "sizing the app"

struct worker
{
    asio::ip::tcp::socket sock_;
    http_io::client ht_; // parser and serializer
    ws_io::client ws_;

    // storage for all operations
    char* buf_;
    std::size_t size_;
};

Websocket server program:

1. configure and apply http settings
2. configure and apply websocket settings
3. allocate max(http-needed, ws-needed)
4. construct parser
    - gets half of the allocation
5. construct serializer
    - gets the other half of the allocation

(receive upgrade request)

6. construct ws_io::server
    - get the full allocation

ws_io::client, ws_io::server

http_io::client, http_io::server



*/
