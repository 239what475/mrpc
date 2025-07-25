import threading
from helloworld import GreeterClient, SayHelloRequest


def main():
    client = GreeterClient("127.0.0.1:8080")

    # 同步调用
    message, err = client.SayHello(SayHelloRequest(name="sync RPC"))
    if err is None:
        print(f"Greeter 1 received: {message}")
    else:
        print(f"Greeter 1 failed: {err}")

    # 异步调用 + Receive
    key, err = client.AsyncSayHello(SayHelloRequest(name="async RPC"))
    if err is None:
        message, err = client.ReceiveSayHello(key)
        if err is None:
            print(f"Greeter 2 received: {message}")
        else:
            print(f"Greeter 2 receive failed: {err}")
    else:
        print(f"Greeter 2 send failed: {err}")

    # 回调调用
    wg = threading.Event()

    def callback(message: str, err: Exception | None):
        if err == None:
            print(f"Greeter 3 received: {message}")
        else:
            print(f"Greeter 3 failed: {err}")
        wg.set()

    client.CallbackSayHello(SayHelloRequest(name="callback RPC"), callback)

    wg.wait()


if __name__ == "__main__":
    main()
