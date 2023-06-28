import socket
import struct
import os
import time

while True:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((os.environ['COMPUTER_IP'], 8080))

    res = s.recv(4096)
    if not res:
        break
    print(res.decode())

    res = s.recv(8)
    if not res:
        break
    x, y = struct.unpack("ii", res)
    print(f'{x} {y}')

    res = s.recv(4*x*y)
    if not res:
        break
    print(f'got {len(res)} bytes')

    s.close()
    time.sleep(10)

print("error, socket did not send data")
s.close()

