import setup
import numpy as np
import pandas as pd
import scipy.stats as sts
import matplotlib.pyplot as plt
from setup import configEnvoirement, readVariables


def data2bytes(rpm, spd, brk):
	b = int(round(rpm))<<16
	b += int(round(spd*1.6))<<8
	b += int(round(brk))

	if(DEBUG):
		print("rpm: %d; spd: %d; brk: %d;" %(rpm, spd, brk))

	return b.to_bytes(4, byteorder='big')


def plotVar(var, name):
	plt.plot(var)
	plt.savefig(('./figs/'+name), dpi=1000)
	plt.cla()
	#plt.figure(figsize = (15, 9.375)).savefig('teste.png', dpi=100)
	return


def smooth(y, box_pts):
	limit = (box_pts - 1)/2
	x = np.linspace(-limit, limit, box_pts)
	box = sts.norm.pdf(x, 0, 2)
	y_smooth = np.convolve(y, box, mode='same')
	return y_smooth


def main():
	plt.rcParams["figure.figsize"] = [30, 22.5]
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
	while _index < len(_log_files):
		log_name = _log_names[_index]
		print("Reading file #%d: %s" %(_index, log_name))
		batch, _batch_size = readVariables(_log_files[_index], _variables, log_name)
		print("Batch size: %dx%d" %(_batch_size[0], _batch_size[1]))

		if(setup.SAVEFIG):
			for i in _variables:
				name = log_name[:len(log_name)-3]+'-'+i+'.png'
				print("Saving %s" %(name))
				plotVar(smooth(batch[i], 31), name)

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

	return 0


if __name__ == "__main__":

	if( setup.Init(ARGS = ["debug", "serial", "tcp", "savefigs"]) == setup.FAIL):
		pass
	else:
		main()
