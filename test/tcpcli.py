# -*- coding: utf-8
import socket 

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# 连接服务端
s.connect(('127.0.0.1', 8000))

# 请求 | 发送数据到服务端
s.sendall(b'hello')

# 响应 | 接受服务端返回到数据
data = s.recv(1024)

print(data) # hello

# 关闭 socket
s.close()
