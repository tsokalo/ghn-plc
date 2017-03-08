#!/usr/bin/python

import pyldpc
import numpy as np

from ghn_ldpc_matrix import *
from encoder import *
from decoder import *

n = 15  # Number of columns
d_v = 4 # Number of ones per column, must be lower than d_c (because H must have more rows than columns)
d_c = 5 # Number of ones per row, must divide n (because if H has m rows: m*d_c = n*d_v (compute number of ones in H))

H = pyldpc.RegularH(n,d_v,d_c)
tG = pyldpc.CodingMatrix(H)
n,k = tG.shape
snr = 8
k,n

v = np.random.randint(2,size=k)
print("The k-bits message v: \n",v)



y = EncodingBPSK(tG,v,snr)
x_decoded = DecodingBPBPSK(H,y,snr,5)
v_received = pyldpc.DecodedMessage(tG,x_decoded)
print("The k-bits decoded message v_recieved is:\n",v_received)



#CodingBPSK()



