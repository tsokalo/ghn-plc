#!/usr/bin/python

import pyldpc
import numpy as np
import copy
import pickle

from multiprocessing import Process, Queue
import time


from ghn_ldpc_matrix import *

num_points = 10

def DoWork(j, k, H, tG, min_snr, max_snr):

	snr = min_snr + (max_snr - min_snr) / float(num_points * (j + 1))

	num_iter=10
	e_bits = 0.0

	for it in range(num_iter):
		#
		# create original message
		#
		v_orig = np.random.randint(2,size=k)

		#
		# encode, add errors and decode the original message
		#
		y = pyldpc.Coding(tG,v_orig,snr)
		x_decoded = pyldpc.Decoding_logBP(H,y,snr,5)
		v_received = pyldpc.DecodedMessage(tG,x_decoded)

		for i in range(len(v_orig)):
			if v_orig[i] != v_received[i]:
				e_bits=e_bits+1

		#print("Number of error bits: \n",e_bits)

	print(str(snr) + " " + str(e_bits / len(v_orig) / num_iter))



procList = []

########################################################################################
#
# create coding matrices
#

#H=GetH_1_2()
#with open('H_matrix_1_2.txt', 'wb') as handle:
#  pickle.dump(H, handle)

H={}
with open('H_matrix_1_2.txt', 'rb') as handle:
    H = pickle.load(handle)


l,m = H.shape
print("H matrix is created with dimensions {}x{}".format(l,m))


#tG = pyldpc.CodingMatrix(H)
#with open('tG_matrix_1_2.txt', 'wb') as handle:
#  pickle.dump(tG, handle)

tG={}
with open('tG_matrix_1_2.txt', 'rb') as handle:
    tG = pickle.load(handle)

n,k = tG.shape
print("\n With G,H you can code messages of {} bits into codewords of {} bits because G's shape is {}\n".format(tG.shape[1],tG.shape[0],tG.T.shape))


for j in range(num_points + 1):
	p = Process(target=DoWork, args=(j, k, H, tG, 3.1, 3.2))
	p.start()
	procList.append(p)				
	time.sleep (2);

for p in procList:
	p.join()	

########################################################################################
#
# create coding matrices
#

#H=GetH_2_3()
#with open('H_matrix_2_3.txt', 'wb') as handle:
#  pickle.dump(H, handle)

with open('H_matrix_2_3.txt', 'rb') as handle:
    H = pickle.load(handle)


l,m = H.shape
print("H matrix is created with dimensions {}x{}".format(l,m))


#tG = pyldpc.CodingMatrix(H)
#with open('tG_matrix_2_3.txt', 'wb') as handle:
#  pickle.dump(tG, handle)

with open('tG_matrix_2_3.txt', 'rb') as handle:
    tG = pickle.load(handle)

n,k = tG.shape
print("\n With G,H you can code messages of {} bits into codewords of {} bits because G's shape is {}\n".format(tG.shape[1],tG.shape[0],tG.T.shape))


for j in range(num_points + 1):
	p = Process(target=DoWork, args=(j, k, H, tG, 4.2, 4.3))
	p.start()
	procList.append(p)				
	time.sleep (2);

for p in procList:
	p.join()	

########################################################################################
#
# create coding matrices
#

#H=GetH_5_6()
#with open('H_matrix_5_6.txt', 'wb') as handle:
#  pickle.dump(H, handle)

with open('H_matrix_5_6.txt', 'rb') as handle:
    H = pickle.load(handle)


l,m = H.shape
print("H matrix is created with dimensions {}x{}".format(l,m))


#tG = pyldpc.CodingMatrix(H)
#with open('tG_matrix_5_6.txt', 'wb') as handle:
#  pickle.dump(tG, handle)

with open('tG_matrix_5_6.txt', 'rb') as handle:
    tG = pickle.load(handle)

n,k = tG.shape
print("\n With G,H you can code messages of {} bits into codewords of {} bits because G's shape is {}\n".format(tG.shape[1],tG.shape[0],tG.T.shape))


for j in range(num_points + 1):
	p = Process(target=DoWork, args=(j, k, H, tG, 5.8, 5.9))
	p.start()
	procList.append(p)				
	time.sleep (2);

for p in procList:
	p.join()	


