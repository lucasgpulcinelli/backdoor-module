import socket
import struct
import os

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((os.environ['COMPUTER_IP'], 8080))

res = s.recv(4096)
if not res:
    quit(-1)
print(res.decode())

res = s.recv(8)
if not res:
    quit(-1)
x, y = struct.unpack("ii", res)

l = 0
final_buf = bytes()
while l < 3*x*y:
    res = s.recv(3*x*y-l)
    if not res:
        break
    l += len(res)
    final_buf += res

f = open("out.ppm", "wb")
f.write(b"P6\n")
f.write(f"{x} {y}\n".encode())
f.write(b"255\n")
f.write(final_buf)

s.close()
