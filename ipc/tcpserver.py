# server.py 
import socket

class tcpServer:
	def __init__(self):
		# create a socket object
		self.serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		# get local machine name
		self.host = socket.gethostname()
		self.port = 5000
		# bind to the port
		self.serversocket.bind((self.host, self.port))


	def sendData(self, data):
		self.clientsocket.send(data)
		return

	def receiveData(self, bytes_quantity):
		return self.clientsocket.recv(bytes_quantity)

	def listen(self, requests):
		# queue up to n requests
		self.serversocket.listen(requests)
		self.clientsocket, self.addr = self.serversocket.accept()
		print("Got a connection from %s" % str(self.addr))