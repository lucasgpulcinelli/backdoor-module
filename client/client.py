import socket
import struct
import os
import time

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((os.environ['COMPUTER_IP'], 8080))

res = bytes()
while len(res) != 4096:
    res += s.recv(4096-len(res))
print(res.decode()[::-1])

res = None
while not res:
    res = s.recv(8)
x, y = struct.unpack("ii", res)

res = bytes()
while len(res) < 3*x*y:
    res += s.recv(3*x*y-len(res))
    time.sleep(0.1)

f = open("out.ppm", "wb")
f.write(b"P6\n")
f.write(f"{x} {y}\n".encode())
f.write(b"255\n")
f.write(res)

s.close()
