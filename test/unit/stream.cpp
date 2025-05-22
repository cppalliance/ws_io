//
// Copyright (c) 2023 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/ws_io
//

#include <boost/ws_proto/handshake.hpp>
#include <boost/rts/context.hpp>
#include <boost/http_proto/response_parser.hpp>
#include <boost/http_io/read.hpp>
#include <boost/asio/write.hpp>
#include "test/unit/server.hpp"
#include "test_suite.hpp"

namespace boost {
namespace ws_io {

void
make_request()
{
}

struct stream_test
{
    void
    run()
    {
        // test::server srv;
        // auto sock = srv.connect();
        // auto req = ws_proto::make_upgrade("/");
        // asio::write(sock, asio::buffer(req.buffer()));
        // srv.run();
        // rts::context ctx;
        // http_proto::parser::config_base cfg;
        // http_proto::install_parser_service(ctx, cfg);
        // http_proto::response_parser pr(ctx);
        // pr.reset();
        // pr.start();
        // http_io::async_read_header(sock, pr, test::success_handler());
        // srv.run();
        // auto res = pr.get();
    }
};

TEST_SUITE(
    stream_test,
    "boost.ws_io.stream");

} // ws_io
} // boost
