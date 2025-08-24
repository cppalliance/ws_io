//
// Copyright (c) 2025 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/ws_io
//

#ifndef BOOST_WS_IO_PEER_HPP
#define BOOST_WS_IO_PEER_HPP

#include <boost/ws_io/detail/config.hpp>
#include <boost/rts/context_fwd.hpp>
#include <type_traits>
#if 0
#include <boost/ws_proto/client.hpp>
#include <boost/http_proto/response_view.hpp>
#include <boost/http_proto/request.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/default_completion_token.hpp>
#include <boost/buffers/const_buffer.hpp>
#include <boost/core/detail/string_view.hpp>
#include <boost/core/span.hpp>
#endif

namespace boost {
namespace ws_io {

template<
    class AsyncStream>
class peer
{
protected:
    AsyncStream stream_;
    rts::context& ctx_;

    bool is_reading_ = false;
    bool is_writing_ = false;

public:
    template<class AsyncStream_>
    peer(
        AsyncStream_&& stream,
        rts::context& ctx)
        : stream_(std::forward<AsyncStream_>(stream))
        , ctx_(ctx)
    {
    }

    /** The type of the underlying stream
    */
    using stream_type = typename
        std::remove_reference<AsyncStream>::type;

    /** The type of executor used by the stream
    */
    using executor_type = decltype(
        std::declval<stream_type>().get_executor());

    /** Return the underlying stream
    */
    AsyncStream&
    next_layer() noexcept
    {
        return stream_;
    }
};

} // ws_io
} // boost

#endif
