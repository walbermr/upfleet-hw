# client.py  
import socket
# create a socket object
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# get local machine name
host = socket.gethostname()
port = 5000
# connection to hostname on the port.
s.connect((host, port))
# Receive no more than 1024 bytes
while True:
	data = s.recv(1024)
	if data:
		print("The time got from the server is %s" %(data.decode('ascii')))
	else:
		break

s.close()