import pyldpc
import numpy as np

n = 25*540  # Number of columns
d_v = 4 # Number of ones per column, must be lower than d_c (because H must have more rows than columns)
d_c = 5 # Number of ones per row, must divide n (because if H has m rows: m*d_c = n*d_v (compute number of ones in H))

H = pyldpc.RegularH(n,d_v,d_c)

print("Regular parity-check matrix H({},{},{}):\n\n".format(n,d_v,d_c),H)

tG = pyldpc.CodingMatrix(H)
print("Transposed Coding Matrix tG that goes with H above is:\n\n",tG)
print("\n With G,H you can code messages of {} bits into codewords of {} bits because G's shape is {}\n".format(tG.shape[1], tG.shape[0],tG.T.shape))

n,k = tG.shape
snr = 8
k,n

v = np.random.randint(2,size=k)
print("The k-bits message v: \n",v)


y = pyldpc.Coding(tG,v,snr)
print("The n-bits message received after transmission:\n\n",y)

x_decoded = pyldpc.Decoding_logBP(H,y,snr,5)
print("The decoded n-bits codeword is:\n",x_decoded)


print("H.x' = ",pyldpc.BinaryProduct(H,x_decoded))



v_received = pyldpc.DecodedMessage(tG,x_decoded)
print("The k-bits decoded message v_recieved is:\n",v_received)
print("\nThe k-bits original message v is:\n",v)




