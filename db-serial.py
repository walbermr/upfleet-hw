import serial, json, h5py, time

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

	#inicializa o arduino
	new_arduino = serial.Serial(port, boud_rate, timeout=timeout)

	if(new_arduino is not None):
		print("Arduino serial ready...")

	return new_arduino, _logs_path, _log_names, _log_files, _variables
		
def readVariables(log_file, variables, filename):
	json = {}
	size = [0, 0]
	for i in range(0, len(variables)):
		try:
			print("Reading " + variables[i]),
			json[variables[i]] = list(log_file[variables[i]])
			size[1] = len(json[variables[i]])
			print("DONE")
		except KeyError:
			print("At file " + filename + " no variable named " + variables[i] + ", skipping.")
		except:
			raise

	size[0] = len(json)

	return json, size


def sendSerialData(arduino, batch, variables, n):
	data = ""
	for i in range(0, len(variables)):
		data += str(batch[variables[i]][n])
		if(i == len(variables) - 1):
			pass
		else:
			data += " "
	
	wb = arduino.write(data)

	if(wb == len(data)):
		print("Data sucessfully sent. %d bytes" %(wb))
	else:
		print("Data size missmatched.")

	return data

def getSerialData(arduino):
	data = arduino.readline()[:-2] #the last bit gets rid of the new-line chars
	return data

def main():
	arduino = None
	batch = {}			#contem todos os valores das variaveis para aquele arquivo de log

	#configuration variables#
	_config_file = "./config.json"
	_logs_path = None
	_log_names = []		#nome dos arquivos de log
	_log_files = []		#descritores dos arquivos de logs
	_variables = []
	##########################

	_index = 0
	try:
		arduino, _logs_path, _log_names, _log_files, _variables = configEnvoirement(_config_file)
	except:
		raise
	
	#envia informacoes sobre a leitura dos sensores
	while True:
		batch, _batch_size = readVariables(_log_files[_index], _variables, _log_names[_index])
		print("Batch size: %dx%d" %(_batch_size[0], _batch_size[1]))

		for i in range(0, _batch_size[1]):
			data_sent = sendSerialData(arduino, batch, _variables, i)
			print("Data sent %s" %(data_sent))
			data_received = getSerialData(arduino)

			if data_received:
				print("Data received %s\n" %(data_received))
				data_received = 0
		
		arduino.close()
		print("Restarting serial...")
		arduino.open()

	return -1


if __name__ == "__main__":
	main()