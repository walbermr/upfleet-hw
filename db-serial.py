import setup, time
import numpy as np
import pandas as pd
import scipy.stats as sts
import matplotlib.pyplot as plt
import matplotlib.mlab as mlab
from setup import configEnvoirement, readVariables
from math import floor, ceil


def data2bytes(rpm, spd, brk):
	if(setup.DEBUG):
		print("rpm: %d; spd: %d; brk: %d;" %(rpm, spd, brk))

	b = int(rpm).to_bytes(2, byteorder='little', signed=True)
	b = b + int(spd).to_bytes(2, byteorder='little', signed=True)
	b = b + int(brk).to_bytes(2, byteorder='little', signed=True)

	return b


def plotVar(name, var_name, dpi, *argv):
	for x, y, trace in argv:
		plt.plot(x, y, trace)

	plt.ylabel(var_name)
	plt.xlabel("Samples")
	plt.savefig(('./figs/'+name), dpi=dpi)
	plt.cla()
	#plt.figure(figsize = (15, 9.375)).savefig('teste.png', dpi=100)
	return

def plotHist(name, var_name, dpi, y):
	n, bins, patches = plt.hist(y, 50, facecolor='blue', alpha=0.75)
	plt.xlabel(var_name)
	plt.ylabel('Frequency')
	plt.savefig(('./figs/'+name), dpi=dpi)
	plt.cla()
	return

def smooth(y, box_pts):
	limit = (box_pts - 1)/2
	x = np.linspace(-limit, limit, box_pts)
	box = sts.norm.pdf(x, 0, 2)
	y_smooth = np.convolve(y, box, mode='same')
	return y_smooth

def decode(data):	#decodifica a informação recebida nos 3 valores de desgaste
	d = int.from_bytes(data, byteorder='little')
	print("DATA: %d" %(d))
	brk = (d >> 4) & 0x3
	clu = (d >> 2) & 0x3
	eng = d & 0x3

	return brk, clu, eng

def plotFracHist(name, var_name, dpi, y, size):
	for i in range(0, ceil(len(y)/size)):
		part_name = var_name+'/'+name+'-part'+str(i)+'hist.png'
		print("Saving: %s" %(part_name))
		plotHist(part_name, var_name, dpi, y[(i*size):((i+1)*size)])

	return



def main():
	plt.rcParams["figure.figsize"] = [6.125, 4.5]
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

		dpi = 100		#DPI do grafico

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
					if(j in ["brake_user", "speed"]):
						if(batch[j][i] > 4096):
							batch[j][i] = 4096
						if(batch[j][i] < 0):
							batch[j][i] = 0

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
				print("Data recv %s" %(data_received))

				if data == data_received:
					print("OK!")

				# if(data_received != str.encode('ok')):
					# print("Data received %s\n" %(data_received))
					# brk, clu, eng = decode(data_received)
					# output["brk"].append(brk)
					# output["clu"].append(clu)
					# output["eng"].append(eng)
		
		print(output)

		if(setup.SAVEFIG):
			size = len(output["eng"])
			if(size > 0):
				copy = round(_batch_size[1]/size)

			outputx = {}

			max_value = max(batch["rpm"])
			outputx["rpm"] = []
			outputx["brk_rate"] = []
			outputx["rpm_rate"] = []

			for i in range(0, size):
				for j in range(0, copy):
					outputx["rpm"].append(floor(output["eng"][i]*max_value/3))
					outputx["brk_rate"].append(floor(output["brk"][i]))
					outputx["rpm_rate"].append(floor(output["clu"][i]))
			
			for i in _variables:

				# the histogram of the data
				name = log_name[:len(log_name)-3]+'-'+i+'-hist.png'
				print("Saving %s" %(name))
				plotHist(name, i, dpi, batch[i])

				name = log_name[:len(log_name)-3]+'-'+i
				#plotFracHist(name, i, dpi, batch[i], 1024)

				name = log_name[:len(log_name)-3]+'-'+i+'.png'
				print("Saving %s" %(name))
				x1 = range(0, len(batch[i]))
				y1 = smooth(batch[i], 31)

				if(i == "rpm"):				#desgaste de motor
					x2 = range(0,len(outputx[i]))
					y2 = outputx[i]

					plotVar(name, i, dpi, (x1, y1, 'b'), (x2, y2, 'r'), (x2, [0]*len(x2), 'r--'), \
					 (x2, [max_value/3]*len(x2), 'r--'), (x2, [2*max_value/3]*len(x2), 'r--'), \
					  (x2, [max_value]*len(x2), 'r--'))

				else:
					plotVar(name, i, dpi, (x1, y1, 'b'))

			if(size > 0):
				var_names = ["rpm_rate", "brk_rate"]
				rates= {"rpm_rate": rpm_rate, "brk_rate": brk_rate}

				for i in var_names:
					y = smooth(rates[i], 31)
					x = range(0, len(y))
					name = log_name[:len(log_name)-3]+'-'+i+'.png'
					print("Saving %s" %(name))

					if(i == "rpm_rate"):	#desgaste de embreagem
						y = (y/np.linalg.norm(y))
						max_value = max(y)
						x2 = range(0,len(outputx[i]))
						y2 = outputx[i]
						plotVar(name, i, dpi, (x, y, 'b'), (x2, [n*max_value/3 for n in y2], 'r'), \
							(x2, [max_value/3]*len(x2), 'r--'), (x2, [2*max_value/3]*len(x2), 'r--'), \
							(x2, [max_value]*len(x2), 'r--'))

					elif(i == "brk_rate"):	#desgaste de freio
						x2 = range(0,len(outputx[i]))
						y2 = outputx[i]

						y = (y1/np.linalg.norm(y1))
						max_value = max(y)
					
						plotVar(name, i, dpi, (x1, y, 'b'), (x2, [n*max_value/3 for n in y2], 'r'), \
							(x2, [max_value/3]*len(x2), 'r--'), (x2, [2*max_value/3]*len(x2), 'r--'), \
							(x2, [max_value]*len(x2), 'r--'), \
							(x1, batch["speed"]/np.linalg.norm(batch["speed"]), 'g'))

					else:
						plotVar(name, i, dpi, (x, y, 'b'))
		_index += 1
		time.sleep(5)

	return 0


if __name__ == "__main__":

	if( setup.Init(ARGS = ["debug", "serial", "tcp", "savefigs"]) == setup.FAIL):
		pass
	else:
		main()
