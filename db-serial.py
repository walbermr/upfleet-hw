import serial, json, h5py, time, sys, math
from ipc.tcpserver import tcpServer

global ARGS, DEBUG, SERIAL, TCP

def configEnvoirement(_config_file):

	json_data = open(_config_file).read()
	data = json.loads(json_data)
	_log_files = []

	boud_rate = int(data["serial_boud_rate"])
	port = str(data["serial_port"])
	timeout = float(data["serial_timeout"])

	_logs_path = data["logs_path"]
	_log_names = data["log_names"]

	#verifica todos os arquivos do dataset e coloca em _log_files
	print("Checking log files")
	for i in range(0, len(_log_names)):
		s = str(_log_names[i])
		_log_names[i] = s
		try:
			print("Log file: " + _log_names[i]),
			_log_files.append(h5py.File(_logs_path + _log_names[i], 'r'))
			print(" -- OK")
		except:
			print(" -- FAIL")
			_log_names.pop(i)
			raise

	#identifica variaveis para serem lidas no batch
	print("\nVariables to be read:")
	_variables = data["variables"]
	if(len(_variables) == 0):
		print("NONE - Make sure to specify variables on config.json.")
		raise NameError('No variables identified on config.json file.')
	else:
		for i in range(0, len(_variables)):
			_variables[i] = str(_variables[i])
			print(_variables[i])
	print('\n')

	#inicializa o arduino
	if(SERIAL):
		new_arduino = serial.Serial(port, boud_rate, timeout=timeout)
		if(new_arduino is not None):
			print("Arduino serial ready...")
		return new_arduino, _logs_path, _log_names, _log_files, _variables, sendSerialData, getSerialData

	#inicializa conexao tcp		
	elif(TCP):
		print("Strating tcp connection.")
		socket = tcpServer()
		socket.listen(1)
		return socket, _logs_path, _log_names, _log_files, _variables, sendTCPData, getTCPData

	#nao retorna nenhum dispotivivo conectado
	else:
		return None, _logs_path, _log_names, _log_files, _variables, None, None
		
def readVariables(log_file, variables, filename):
	json = {}
	size = [0, 0]
	for i in range(0, len(variables)):
		try:
			print("Reading " + variables[i]),
			json[variables[i]] = list(log_file[variables[i]])
			size[1] = len(json[variables[i]])
			print("DONE")

			if(DEBUG):
				print(json[variables[i]])

		except KeyError:
			print("At file " + filename + " no variable named " + variables[i] + ", skipping.")
		except:
			raise

	size[0] = len(json)

	return json, size

##terminar
def data2bytes(rpm, spd, brk):
	b = int(round(rpm))<<8
	b += int(round(spd*3.6))<<2
	b += int(round(brk))

	if(DEBUG):
		print("rpm: %d; spd: %d; brk: %d;" %(rpm, spd, brk))

	return b.to_bytes(4, byteorder='big')

##terminar
def sendSerialData(arduino, data):
	wb = arduino.write(data)

	if(DEBUG):
		if(wb == 4):
			print("%d bytes sent." %(wb))
		else:
			print("Data size missmatched. Data size %d." %(wb))

	return

def getSerialData(arduino):
	data = arduino.readline()[:-2] #the last bit gets rid of the new-line chars
	return data

def restartSerial(arduino):
	arduino.close()
	print("Restarting serial...")
	arduino.open()
	return


def sendTCPData(socket, data):
	socket.sendData(data);
	return

def getTCPData(socket):
	data = socket.receiveData(1024)
	return

def main():
	batch = {}				#contem todos os valores das variaveis para aquele arquivo de log

	device = None 			#dispositivo pelo qual se enviara os dados
	send_function = None 	#funcao de envio do dispositivo
	recv_function = None 	#funcao de recebimento do dispositivo

	#configuration variables#
	_config_file = "./config.json"
	_logs_path = None
	_log_names = []			#nome dos arquivos de log
	_log_files = []			#descritores dos arquivos de logs
	_variables = []
	#########################

	_index = 0
	try:
		device, _logs_path, _log_names, _log_files, _variables, send_function, recv_function = configEnvoirement(_config_file)
			
	except:
		raise
	
	#envia informacoes sobre a leitura dos sensores
	while True:
		print("Reading file #%d: %s" %(_index, _log_names[_index]))
		batch, _batch_size = readVariables(_log_files[_index], _variables, _log_names[_index])
		print("Batch size: %dx%d" %(_batch_size[0], _batch_size[1]))

		if(device != None):
			for i in range(0, _batch_size[1]):
				d = []

				for j in range(0, len(_variables)):
					d.append(batch[_variables[j]][i])

				data = data2bytes(d[0], d[1], d[2])
				send_function(device, data)
				print("Data sent %s" %(data))

				data_received = recv_function(device)

				if data_received:
					print("Data received %s\n" %(data_received))
					data_received = 0
				else:
					print('\n')

		_index += 1

	return -1


if __name__ == "__main__":
	ARGS = ["debug", "serial", "tcp"]
	DEBUG = False
	SERIAL = False

	args = sys.argv[1:]	#captura os parametros para execucao

	for a in args:
		if(a == ARGS[0]):
			DEBUG = True
		elif(a == ARGS[1]):
			SERIAL = True
		elif(a == ARGS[2]):
			TCP = True

	if(SERIAL and TCP):
		print("Can't send through serial and tcp at the same time.")

	else:
		main()
