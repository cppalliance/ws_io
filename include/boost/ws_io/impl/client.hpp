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
#include <boost/asio/coroutine.hpp>

namespace boost {
namespace ws_io {

//------------------------------------------------

template<class AsyncStream>
template<class Handler>
class client<AsyncStream>::
    handshake_op
    : public asio::coroutine
{
    client<AsyncStream>& cs_;
    Handler h_;

public:
    template<class Handler_>
    handshake_op(
        client<AsyncStream>& cs,
        Handler_&& h,
        core::string_view host,
        core::string_view target)
        : cs_(cs)
        , h_(std::forward<Handler_>(h))
    {
        (void)host;
        (void)target;
    }

};

//------------------------------------------------

template<class AsyncStream>
struct client<AsyncStream>::
    run_handshake_op
{
    client<AsyncStream>& self;

    using executor_type = typename
        client<AsyncStream>::executor_type;

    executor_type
    get_executor() const noexcept
    {
        return self.next_layer().get_executor();
    }

    template<class HandshakeHandler>
    void operator()(
        HandshakeHandler&& h,
        core::string_view host,
        core::string_view target
        /*,request_type&& req
        ,detail::sec_ws_key_type key
        ,response_type* res_p*/
        )
    {
        handshake_op<typename std::decay<
            HandshakeHandler>::type>(
                self,
                std::forward<HandshakeHandler>(h),
                host,
                target);
    }
};

//------------------------------------------------

template<class AsyncStream>
client<AsyncStream>::
client(
    AsyncStream& stream,
    rts::context& ctx)
    : stream_(stream)
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
    return asio::async_initiate<
        HandshakeHandler,
        void(system::error_code, http_proto::response_view)>(
            run_handshake_op{*this},
            handler,
            host,
            target);
}

} // ws_io
} // boost

#endif
