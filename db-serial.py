import setup
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from setup import configEnvoirement, readVariables


def data2bytes(rpm, spd, brk):
	b = int(round(rpm))<<16
	b += int(round(spd*3.6))<<8
	b += int(round(brk))

	if(DEBUG):
		print("rpm: %d; spd: %d; brk: %d;" %(rpm, spd, brk))

	return b.to_bytes(4, byteorder='big')


def PlotVar(var):
	plt.plot(var)
	plt.show()
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
		PlotVar(batch["rpm"])

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

	if( setup.Init(ARGS = ["debug", "serial", "tcp"]) == setup.FAIL):
		pass
	else:
		main()
