import signal
import threading
import sys
from helloworld import GreeterServer, GreeterService, SayHelloRequest, SayHelloResponse
import mrpc  # Import mrpc to get MrpcError

# 继承GreeterService并实现SayHello方法
class GreeterServiceImpl(GreeterService):
    def SayHello(self, request: SayHelloRequest, response: SayHelloResponse) -> mrpc.MrpcError | None:
        response.message = f"Hello {request.name}"
        return None

# 信号处理函数
stop_event = threading.Event()

def signal_handler(sig, frame):
    print(f"收到信号 {sig}，准备停止服务器...")
    stop_event.set()

def main():
    server_address = "0.0.0.0:8080"
    server = GreeterServer(server_address)
    service = GreeterServiceImpl()
    server.RegisterService(service)
    err = server.Start()
    if err is not None:
        print(f"服务器启动失败: {err.message}")
        sys.exit(1)
    
    print(f"服务器成功启动，监听地址: {server_address}")
    
    # 设置信号处理
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    # 等待停止信号
    try:
        stop_event.wait()
    except KeyboardInterrupt:
        print("收到键盘中断，准备停止服务器...")
        stop_event.set()
    
    # 停止服务器
    server.Stop()
    print("服务器已停止")
    sys.exit(0)

if __name__ == "__main__":
    main()