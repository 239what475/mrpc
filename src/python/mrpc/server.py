import ctypes
import threading
from .defs import request_handler, lib_server, MrpcCall
from .server_utils import MrpcService
from .status import MrpcError

UNKNOWN = "UNKNOW_FUNC"

g_callbacks = {}
g_callback_mutex = threading.Lock()

def RegisterRpcCallback(method: str, cb):
    with g_callback_mutex:
        g_callbacks[method] = cb
        
@request_handler
def ServerCallback(method: bytes, key: bytes, request: bytes, source: bytes):
    method_str = method.decode()
    key_str = key.decode()
    request_str = request.decode()
    source_str = source.decode()
    with g_callback_mutex:
        cb = g_callbacks.get(method_str)
    if cb:
        cb(key_str, request_str, source_str)
    else:
        print(f"No handler for method: {method_str}")

class Server:
    def __init__(self, addr: str):
        self.server = lib_server.mrpc_create_server(addr.encode(), ServerCallback)
        self.services = []

    def RegisterService(self, service: MrpcService):
        self.services.append(service)
        for method, handler in service.GetHandlers().items():
            def make_cb(handler):
                def cb(key, request, source):
                    result = []
                    err = handler.Run(request, result)
                    response = result[0] if result else (str(err) if err else "")
                    call = MrpcCall.NewMrpcCall(key, response)
                    lib_server.mrpc_send_reponse(self.server, ctypes.byref(call), source.encode())
                return cb
            RegisterRpcCallback(method, make_cb(handler))

    def Start(self) -> MrpcError | None:
        # Register UNKNOWN callback before starting
        def unknown_cb(key, request, source):
            call = MrpcCall.NewMrpcCall(key, "no such func")
            lib_server.mrpc_send_reponse(self.server, ctypes.byref(call), source.encode())
        RegisterRpcCallback(UNKNOWN, unknown_cb)

        ret = lib_server.mrpc_start_server(self.server)
        if ret == 0:
            return None
        else:
            return MrpcError.MRPC_SEND_FAILURE

    def Stop(self):
        lib_server.mrpc_destroy_server(self.server)
