import matplotlib.pyplot as plt
import pandas as pd
import warnings # current version of seaborn generates a bunch of warnings that we'll ignore
warnings.filterwarnings("ignore")
import seaborn as sns
sns.set(color_codes=True)


def plot_var(name, var_name, dpi, *argv):
	for x, y, trace in argv:
		plt.plot(x, y, trace)

	plt.ylabel(var_name)
	plt.xlabel("Samples")
	plt.savefig(('./figs/'+name), dpi=dpi)
	plt.cla()
	return

def tri_d_plot(name, var, dpi):
	print("Plotting 3D chart.")
	fig = plt.figure()
	ax = fig.add_subplot(111, projection='3d')
	ax.scatter(var[0], var[1], var[2])

	ax.set_xlabel('rpm')
	ax.set_ylabel('speed')
	ax.set_zlabel('brake_user')

	plt.savefig(('./figs/'+name), dpi=dpi)
	#plt.show()
	return

def scatterplot_matrix(name, dset, hue, keys, dpi):
	dset = pd.DataFrame.from_dict(dset, orient='columns')
	print("Plotting dataset scatterplots.")
	g = sns.pairplot(dset, hue=hue, vars=keys)
	#g.fig.get_children()[-1].set_bbox_to_anchor((1.1, 0.5, 0, 0))
	g.savefig("./figs/"+name, dpi=dpi)
	return

def plot_histogram(name, var_name, dpi, y):
	n, bins, patches = plt.hist(y, 50, facecolor='blue', alpha=0.75)
	plt.xlabel(var_name)
	plt.ylabel('Frequency')
	plt.savefig(('./figs/'+name), dpi=dpi)
	plt.cla()
	return

def plot_fraction_histogram(name, var_name, dpi, y, size):
	for i in range(0, ceil(len(y)/size)):
		part_name = var_name+'/'+name+'-part'+str(i)+'hist.png'
		print("Saving: %s" %(part_name))
		plot_histogram(part_name, var_name, dpi, y[(i*size):((i+1)*size)])

	return