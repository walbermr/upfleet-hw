import setup, time
import numpy as np
import pandas as pd
import scipy.stats as sts
import matplotlib.pyplot as plt
from setup import configEnvoirement, readVariables


def data2bytes(rpm, spd, brk):
	if(setup.DEBUG):
		print("rpm: %d; spd: %d; brk: %d;" %(rpm, spd, brk))

	b = int(rpm).to_bytes(2, byteorder='big', signed=False)
	b = b + int(spd).to_bytes(2, byteorder='big', signed=True)
	b = b + int(brk).to_bytes(2, byteorder='big', signed=False)

	return b


def plotVar(var, name, dpi):
	plt.plot(var)
	plt.savefig(('./figs/'+name), dpi=dpi)
	plt.cla()
	#plt.figure(figsize = (15, 9.375)).savefig('teste.png', dpi=100)
	return


def smooth(y, box_pts):
	limit = (box_pts - 1)/2
	x = np.linspace(-limit, limit, box_pts)
	box = sts.norm.pdf(x, 0, 2)
	y_smooth = np.convolve(y, box, mode='same')
	return y_smooth

def decode(data):	#decodifica a informação recebida nos 3 valores de desgaste
	d = int.from_bytes(data, byteorder='big')
	print("DATA: %d" %(d))
	brk = (d >> 4) & 0x3
	clu = (d >> 2) & 0x3
	eng = d & 0x3

	return brk, clu, eng



def main():
	plt.rcParams["figure.figsize"] = [6, 4.5]
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
		rpm_rate = []	#derivada do rpm
		brk_rate = []	#derivada do freio

		last_rpm = 0	#ultima leitura de rpm
		last_brk = 0	#ultima leitura de freio

		output = {"brk":[], "clu":[], "eng":[]}
		log_name = _log_names[_index]
		print("Reading file #%d: %s" %(_index, log_name))
		batch, _batch_size = readVariables(_log_files[_index], _variables, log_name)
		print("Batch size: %dx%d" %(_batch_size[0], _batch_size[1]))

		if(device != None):
			for i in range(0, _batch_size[1]):
				data_received = 0
				d = []

				for j in _variables:
					#converte de mph para kph
					#if(j == "spd"):
					#	batch[j][i] = batch[j][i] * 1.6
					#existem problemas nas leituras do freio, isso acaba com tudo
					if(j == "brake_user"):
						if(batch[j][i] > 4096):
							batch[j][i] = 4096
						if(batch[j][i] < 0):
							batch[j][i] = 0;

					d.append(batch[j][i])

				#cria as variaveis de derivada
				rpm_rate.append(batch["rpm"][i] - last_rpm)
				brk_r = batch["brake_user"][i] - last_brk
				if(brk_r > 300):
					brk_rate.append(300)
				elif(brk_r < -300):
					brk_rate.append(-300)
				else:
					brk_rate.append(brk_r)

				last_rpm = batch["rpm"][i]
				last_brk = batch["brake_user"][i]
				#################################

				data = data2bytes(d[0], d[1], d[2])
				send_function(device, data)
				print("Data sent %s" %(data))

				data_received = recv_function(device)

				if(data_received != str.encode('ok')):
					print("Data received %s\n" %(data_received))
					brk, clu, eng = decode(data_received)
					output["brk"].append(brk)
					output["clu"].append(clu)
					output["eng"].append(eng)
		
		print(output)

		if(setup.SAVEFIG):
			for i in _variables:
				name = log_name[:len(log_name)-3]+'-'+i+'.png'
				print("Saving %s" %(name))
				plotVar(smooth(batch[i], 31), name, 100)

			var_names = ["rpm_rate", "brk_rate"]
			rates= {"rpm_rate": rpm_rate, "brk_rate": brk_rate}

			for i in var_names:
				name = log_name[:len(log_name)-3]+'-'+i+'.png'
				print("Saving %s" %(name))
				plotVar(smooth(rates[i], 31), name, 100)

		_index += 1
		time.sleep(10)

	return 0


if __name__ == "__main__":

	if( setup.Init(ARGS = ["debug", "serial", "tcp", "savefigs"]) == setup.FAIL):
		pass
	else:
		main()
