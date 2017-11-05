import serial, json, h5py, time, math, sys
from ipc.tcpserver import tcpServer

FAIL = False

def Init(ARGS):
	global DEBUG, SERIAL, TCP, SAVEFIG

	DEBUG = False
	SERIAL = False
	TCP = False
	SAVEFIG = False

	args = sys.argv[1:]	#captura os parametros para execucao

	for a in args:
		if(a == ARGS[0]):
			DEBUG = True
		elif(a == ARGS[1]):
			SERIAL = True
		elif(a == ARGS[2]):
			TCP = True
		elif(a == ARGS[3]):
			SAVEFIG = True

	if(SERIAL and TCP):
		print("Can't send through serial and tcp at the same time.")
		return False
	else:
		return True

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
		new_arduino = serial.Serial(port, boud_rate)#, timeout=timeout)
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

def sendSerialData(arduino, data):
	wb = arduino.write(data)

	# if(DEBUG):
	# 	if(wb == 6):
	# 		print("%d bytes sent." %(wb))
	# 	else:
	# 		print("Data size missmatched. Data size %d." %(wb))

	return wb

def getSerialData(arduino):
	return arduino.read(6)

def restartSerial(arduino):
	arduino.close()
	print("Restarting serial...")
	arduino.open()
	return


def sendTCPData(socket, data):
	socket.sendData(data);
	return

def getTCPData(socket):
	data = socket.receiveData(2)
	return data