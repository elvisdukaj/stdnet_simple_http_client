// http-server-template.cpp                                           -*-C++-*-
// ----------------------------------------------------------------------------
//
//  Copyright (c) 2024 Dietmar Kuehl http://www.dietmar-kuehl.de
//
//  Licensed under the Apache License Version 2.0 with LLVM Exceptions
//  (the "License"); you may not use this file except in compliance with
//  the License. You may obtain a copy of the License at
//
//    https://llvm.org/LICENSE.txt
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
// ----------------------------------------------------------------------------

#include <cstddef>
#include <fmt/base.h>
#include <stdnet/buffer.hpp>
#include <stdnet/internet.hpp>
#include <stdnet/socket.hpp>
#include <stdnet/timer.hpp>

#include <stdexec/execution.hpp>
#include <exec/async_scope.hpp>
#include <exec/task.hpp>
#include <exec/when_any.hpp>

#include <exception>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/format.h>

using namespace std::chrono_literals;
using namespace std::string_view_literals;

// ----------------------------------------------------------------------------
// stdnet::ip::tcp::acceptor
// stdnet::ip::tcp::endpoint
// 200 OK
// 301 Moved Permanently (Location: URL)
// 400 Bad Request
// 404 Not Found
// 500 Internal Server Error
// 501 Not Implemented

exec::task<void> make_client(auto& context, bool keep_alive)
{
  try {
    using stream_socket = stdnet::basic_stream_socket<stdnet::ip::tcp>;
    // google.com -> 172.217.23.206
    stream_socket client(context, stdnet::ip::tcp::endpoint(stdnet::ip::address_v4::loopback(), 12345));
    co_await stdnet::async_connect(client);

    std::string request("GET / HTTP/1.1\r\n");
    if (keep_alive) {
      request.append("Connection: Keep-Alive\r\n");
      request.append("GET / HTTP/1.1");
    }
    request.append("\r\n\r\n");

    co_await stdnet::async_send(client, stdnet::buffer(request));
    
    std::vector<char> b(10000);
    auto n = co_await stdnet::async_receive(client, stdnet::buffer(b));
    fmt::println("Recieved=>>{}", std::string_view{b.data(), n});
  } catch(const std::exception& exc) {
    fmt::println("An error occured: {}", exc.what());
  }
}

int main(int argc, char* argv[])
{
    bool keep_alive = false;
    if (argc > 1) {
      keep_alive = std::stoi(argv[1]) != 0;
    }

    stdnet::io_context context;
    exec::async_scope scope;
    scope.spawn(make_client(context, keep_alive));
    context.run();
    stdexec::sync_wait(scope.on_empty());
}
