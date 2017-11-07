import serial
import time
import random

ard = serial.Serial('COM7', 9600, timeout = 0.6)
ard.write(int(5).to_bytes(6, 'little'))

for i in range(2000000):
	s = int(i).to_bytes(6, 'little')

	ard.write(s)
	print("SEND: %s" %(s))

	# for j in range(6):
	# 	r = ard.read(1)
	# 	print(r)

	# print("------------------------------")

	# r = ard.read(2)
	# print("RECV: %s" %(int.from_bytes(r, 'little')))
	r = ard.read(6)
	print("RECV: %s" %(r))

	if (s == r):
		print("OK!!")
	# print("send: %d - recv: %d" %(t2-t1, t3-t2))
