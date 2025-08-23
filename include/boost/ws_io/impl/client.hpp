//
// Copyright (c) 2025 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/ws_io
//

#ifndef BOOST_WS_IO_IMPL_CLIENT_HPP
#define BOOST_WS_IO_IMPL_CLIENT_HPP

#include <boost/asio/async_result.hpp>
#include <boost/http_proto/response_view.hpp>
#include <boost/ws_proto/handshake.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/write.hpp>

namespace boost {
namespace ws_io {

//------------------------------------------------

template<class AsyncStream>
class client<AsyncStream>::
    handshake_op
    : public asio::coroutine
{
    client<AsyncStream>& cs_;
    http_proto::request req_;

public:
    handshake_op(
        client<AsyncStream>& cs,
        core::string_view host,
        core::string_view target)
        : cs_(cs)
        , req_(ws_proto::make_upgrade(host, target))
    {
    }

    template<class Self>
    void
    operator()(
        Self& self,
        system::error_code ec = {},
        std::size_t bytes_transferred = 0)
    {
        (void)bytes_transferred;
        BOOST_ASIO_CORO_REENTER(*this)
        {
            BOOST_ASIO_CORO_YIELD
            {
                BOOST_ASIO_HANDLER_LOCATION((
                    __FILE__, __LINE__,
                    "async_write"));
                asio::async_write(
                    cs_.next_layer(),
                    asio::buffer(req_.buffer()),
                    std::move(self));
            }
            if(ec.failed())
                goto upcall;
            // fallthrough
        upcall:
            self.complete(ec, {});
        }
    }
};

//------------------------------------------------

template<class AsyncStream>
template<class AsyncStream_>
client<AsyncStream>::
client(
    AsyncStream_&& stream,
    rts::context& ctx)
    : stream_(std::forward<AsyncStream_>(stream))
    , ctx_(ctx)
{
}

template<class AsyncStream>
template<
    BOOST_ASIO_COMPLETION_TOKEN_FOR(void(
        ::boost::system::error_code,
        ::boost::http_proto::response_view)) HandshakeHandler,
    class Decorator>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(HandshakeHandler, void(
    ::boost::system::error_code,
    ::boost::http_proto::response_view))
client<AsyncStream>::
async_handshake(
    core::string_view host,
    core::string_view target,
    Decorator decorator,
    HandshakeHandler&& handler)
{
    (void)host;
    (void)target;
    (void)decorator;
    return asio::async_compose<
        HandshakeHandler,
        void(system::error_code, http_proto::response_view)>(
        handshake_op( *this, host, target ),
        handler,
        stream_); // or *this
}

} // ws_io
} // boost

#endif
