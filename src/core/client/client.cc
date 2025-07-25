#include "src/core/client/client.h"

#define MRPC_API

namespace mrpc {
mrpc_status Client::Send(mrpc_call *call) {
  std::call_once(channel_flag_, &Client::_init_channel, this);
  return connection_->Send(call);
}

} // namespace mrpc

MRPC_API mrpc_client *mrpc_create_client(const char *addr,
                                         response_handler handler) {
  return (new mrpc::Client(addr, handler))->c_ptr();
}

MRPC_API void mrpc_destroy_client(mrpc_client *client) {
  delete mrpc::Client::FromC(client);
}

MRPC_API mrpc_status mrpc_send_request(mrpc_client *client, mrpc_call *call) {
  return mrpc::Client::FromC(client)->Send(call);
}
