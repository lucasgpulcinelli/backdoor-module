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

    l = 0
    while l < 3*x*y:
        res = s.recv(3*x*y-l)
        if not res:
            break
        l += len(res)
        print(f'got {l} bytes')

    s.close()
    time.sleep(10)

print("error, socket did not send data")
s.close()

