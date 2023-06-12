import socket
import os

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((os.environ['COMPUTER_IP'], 8080))
while True:
    b = s.recv(1024)
    if not b:
        break
    print(b.decode(), end='')

s.close()

