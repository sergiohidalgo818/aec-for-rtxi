
###########################################
###	Active Electrode Compensation Train ###
###########################################
import h5py
import matplotlib.pyplot as plt
import numpy as np
import sys
import argparse 

import aec

#####################
###	Aux Functions ###
#####################
def get_date(f):
	struct = '/'+trial+'/Date'
	data = f[struct][()]
	data=data.decode('UTF-8')
	data = data[data.rfind('T')+1:]
	data=data.split(':')

	date = data[0]+"h"+data[1]+"m"+data[2]+"s"
	return date

#######################
### Argument Parser ###
#######################
ap = argparse.ArgumentParser()

ap.add_argument("-p", "--path", required=True,
	help="Path to the file to analyze")

ap.add_argument("-m","--mode",required=True,
	help="Type of recording intra: only 0 and 1; extra: intra and extracellular 0 1 2 3; manual: specify -c and -he parameters")

ap.add_argument("-c","--cols",required=False,
	help="Index to the elements in the trial separated by space")

ap.add_argument("-he","--headers",required=False,
	help="Header to the file separated by space")

args = vars(ap.parse_args())

############
### Path ###
############
path = args['path']
filename=path
file = path[path.rfind("/")+1:]
path = path[:path.rfind("/")+1]
print(path,file)

####################
### File details ###
####################
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

##################
### Open file  ###
##################
try:
	f = h5py.File(filename, 'r')		
except:
	print("File not valid")
	exit()

#################################
### Trial Selection for Train ###
#################################
i=4
trial = "Trial"+str(i)
struct = '/'+trial+'/Synchronous Data/Channel Data'
dset = f[struct]
# data = dset.value #deprecated
data = dset[()]
res = data[:,columns].T

c_whitenoise = res[0]
v_recorded   = res[1]

#############
### Train ###
#############
k = aec.full_kernel(v_recorded, c_whitenoise, 200)
ke = aec.electrode_kernel(k,100)

##########################
### Compensation Train ###
##########################
v_compensed = aec.AEC_compensate(v_recorded, c_whitenoise, ke)

plt.title("Train")
plt.plot(v_compensed , alpha=1.0, label='Compensated V')
plt.plot(v_recorded  , alpha=0.8, label='Recorded V')
plt.plot(c_whitenoise, alpha=0.3, label='Inserted C')
plt.legend()
plt.show()

###################
### Save Kernel ###
###################
ke.tofile('aec_kernel.txt', sep='\n', format='%s')

#################################
### Trial Selection for Test ###
#################################
i=5
trial = "Trial"+str(i)
struct = '/'+trial+'/Synchronous Data/Channel Data'
dset = f[struct]
# data = dset.value #deprecated
data = dset[()]
res = data[:,:].T

c_periodicnoise = res[0]
v_recorded      = res[1]

#########################
### Compensation Test ###
#########################
v_compensed = aec.AEC_compensate(v_recorded, c_periodicnoise, ke)

plt.title("Test")
plt.plot(v_compensed    ,  alpha=1.0, label='Compensated V')
plt.plot(v_recorded     , alpha=0.8, label='Recorded V')
plt.plot(c_periodicnoise, alpha=0.3, label='Inserted C')
plt.legend()
plt.show()

#############
### Check ###
#############
v_compensed = aec.AEC_compensate([0.1], [1,2,3,4], ke)
print('\nCheck value:')
print(v_compensed)
