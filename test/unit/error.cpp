//
// Copyright (c) 2023 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/ws_io
//

// Test that header file is self-contained.
#include <boost/ws_io/error.hpp>

#include "test_suite.hpp"

namespace boost {
namespace ws_io {

struct error_test
{
    void
    run()
    {
    }
};

TEST_SUITE(
    error_test,
    "boost.ws_io.error");

} // ws_io
} // boost
