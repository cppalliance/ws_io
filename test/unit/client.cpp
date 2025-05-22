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
        rts::context ctx;
        http_proto::parser::config_base cfg;
        http_proto::install_parser_service(ctx, cfg);

        test::server srv;
        auto sock = srv.connect();
        client<test::socket_type> cs(sock, ctx);
        cs.async_handshake(
            "localhost",
            "/",
            [](http_proto::request&)
            {
            },
            test::success_handler());
        //srv.run();
    }
};

TEST_SUITE(
    client_test,
    "boost.ws_io.client");

} // ws_io
} // boost
