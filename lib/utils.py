import setup

def data_to_bytes(rpm, spd, brk):
	if(setup.DEBUG):
		print("rpm: %d; spd: %d; brk: %d;" %(rpm, spd, brk))

	b = int(rpm).to_bytes(2, byteorder='big', signed=False)
	b = b + int(spd).to_bytes(2, byteorder='big', signed=True)
	b = b + int(brk).to_bytes(2, byteorder='big', signed=False)

	return b

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

def tame_dset(dset, sizes, names):	#existem problemas nas leituras do freio, isso acaba com tudo

	for i in range(0, sizes[1]):
		for j in names:
			if(j == "brake_user"):
				if(dset[j][i] > 4096):
					dset[j][i] = 4096
				if(dset[j][i] < 0):
					dset[j][i] = 0;
	return dset