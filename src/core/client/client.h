#pragma once

#include "src/core/client/connection.h"
#include "src/core/common/c_cpp_trans.h"

#include "mrpc/mrpc.h"

#include <iostream>
#include <thread>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

namespace mrpc {
class Client : public CppImplOf<Client, mrpc_client> {
public:
  Client(const std::string &addr, response_handler handler)
      : io_(), work_guard_(boost::asio::make_work_guard(io_)),
        io_thread_(std::thread([this] { io_.run(); })), target_(addr),
        handler_(handler) {}

  ~Client() {
    std::cout << "clear rpc client" << std::endl;
    work_guard_.reset();
    io_.stop();
    if (io_thread_.joinable()) {
      io_thread_.join();
    }
  }

  mrpc_status Send(mrpc_call *call);

private:
  void _init_channel() {
    connection_ = client::Connection::Create(io_, target_, handler_);
    connection_->Connect();
  }

private:
  boost::asio::io_context io_;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
      work_guard_;
  std::thread io_thread_;
  client::Target target_;
  std::shared_ptr<client::Connection> connection_;
  std::once_flag channel_flag_;
  response_handler handler_;
};
} // namespace mrpc