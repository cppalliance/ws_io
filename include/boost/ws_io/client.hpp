//
// Copyright (c) 2025 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/ws_io
//

#ifndef BOOST_WS_IO_CLIENT_HPP
#define BOOST_WS_IO_CLIENT_HPP

#include <boost/ws_io/detail/config.hpp>
#include <boost/ws_io/peer.hpp>
#include <boost/ws_proto/client.hpp>
#include <boost/http_proto/response_view.hpp>
#include <boost/http_proto/request.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/default_completion_token.hpp>
#include <boost/buffers/const_buffer.hpp>
#include <boost/core/detail/string_view.hpp>
#include <boost/core/span.hpp>
#include <type_traits>

namespace boost {
namespace ws_io {

struct null_decorator
{
    void operator()(...) const noexcept
    {
    }
};

struct frame
{
    ws_proto::frame_type kind;
    buffers::const_buffer data;
};

struct read_results
{
    span<buffers::const_buffer> messages;
};

//------------------------------------------------

/** A websocket client session
*/
template<
    class AsyncStream>
class client : public peer<AsyncStream>
{
public:
    template<class AsyncStream_>
    client(
        AsyncStream_&& stream,
        rts::context& ctx)
        : peer<AsyncStream>(
            std::forward<AsyncStream_>(stream),
            ctx)
    {
    }

    /** Perform the websocket handshake
    */
    template<
        BOOST_ASIO_COMPLETION_TOKEN_FOR(void(
            ::boost::system::error_code,
            ::boost::http_proto::response_view)) HandshakeHandler =
            asio::default_completion_token_t<executor_type>,
        class Decorator = null_decorator
    >
    BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(HandshakeHandler, void(
        ::boost::system::error_code,
        ::boost::http_proto::response_view))
    async_handshake(
        core::string_view host,
        core::string_view target,
        HandshakeHandler&& handler =
            asio::default_completion_token_t<executor_type>{},
        Decorator decorator = {}
        );

    /** Write a complete message
    */
    template<
        class ConstBufferSequence,
        BOOST_ASIO_COMPLETION_TOKEN_FOR(void(
            ::boost::system::error_code,
            std::size_t)) WriteHandler =
            asio::default_completion_token_t<executor_type>
    >
    BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(WriteHandler, void(
        ::boost::system::error_code,
        std::size_t))
    async_write(
        ConstBufferSequence const& data,
        WriteHandler&& handler =
            asio::default_completion_token_t<executor_type>{}
        );

private:
    class handshake_op;
    class write_op;
};

} // ws_io
} // boost

#include <boost/ws_io/impl/client.hpp>

#endif
