
#################################################################################################
###	Parse h5 file to plain text, each Trial is saved in a different file.
#################################################################################################

import h5py
import matplotlib.pyplot as plt
import numpy as np
import sys
import argparse 


def get_date(f):
	struct = '/'+trial+'/Date'
	data = f[struct][()]
	data=data.decode('UTF-8')
	data = data[data.rfind('T')+1:]
	data=data.split(':')

	date = data[0]+"h"+data[1]+"m"+data[2]+"s"

	return date


ap = argparse.ArgumentParser()

ap.add_argument("-p", "--path", required=True, help="Path to the file to analyze")
ap.add_argument("-m","--mode",required=True,
	help="Type of recording intra: only 0 and 1; extra: intra and extracellular 0 1 2 3; manual: specify -c and -he parameters" )
ap.add_argument("-c","--cols",required=False,help="Index to the elements in the trial separated by space")
ap.add_argument("-he","--headers",required=False,help="Header to the file separated by space")

args = vars(ap.parse_args())

path = args['path']
filename=path
file = path[path.rfind("/")+1:]
path = path[:path.rfind("/")+1]
print(path,file)

mode = args['mode']

if mode =='intra':
	headers = 'intra0 intra1'
	columns = (0,1) #Corresponding value in HDF5 group!!
elif mode == 'extra':
	headers = 'intra0 intra1 extra2 extra3'
	columns = (0,1,2,3) #Corresponding value in HDF5 group!!
elif mode == 'manual':
	columns = tuple([int(col) for col in args['cols'].split()])
	headers = args['headers']
else:
	print("Unrecognized mode. Use --help for more info.")
	exit()

print(columns,headers)

#Open file 
try:
	f = h5py.File(filename, 'r')		
except:
	print("File not valid")
	exit()

i=1
while 1:
	trial = "Trial"+str(i)

	struct = '/'+trial+'/Synchronous Data/Channel Data'

	try:
		dset = f[struct]
		# data = dset.value #deprecated
		data = dset[()]
	except KeyError as e:
		if 'Channel Data' in e.args[0]:
			print("Skiping Trial %d"%i)
			i+=1
			continue
		else:
			print("No trials left. %d files generated"%i)
			break

	try:
		res = data[:,columns]
	except Exception as e:
		print("Warning: ", e.args)
		res = np.array([])

	date = get_date(f)

	date_time=path+date+"_"+trial+"_"+file[:-3]
	name = date_time+".asc"
	print(name,res.shape)
	i+=1
												
	np.savetxt(name, res, delimiter=' ',header=date_time+"\n"+headers)
