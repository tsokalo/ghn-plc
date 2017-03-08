#!/usr/bin/python

import pyldpc
from pyldpc.ldpcalgebra import*
import scipy
import copy
import numpy as np

from ghn_ldpc_matrix import *

def DecodingBPBPSK(H,y,SNR,max_iter=1):

    """ Decoding function using Belief Propagation algorithm.
        
        
        IMPORTANT: H can be scipy.sparse.csr_matrix object to speed up calculations if n > 1000 highly recommanded. 
    -----------------------------------
    Parameters:
    
    H: 2D-array (OR scipy.sparse.csr_matrix object) Parity check matrix, shape = (m,n) 

    y: n-vector recieved after transmission in the channel. (In general, returned 
    by Coding Function)


    Signal-Noise Ratio: SNR = 10log(1/variance) in decibels of the AWGN used in coding.
    
    max_iter: (default = 1) max iterations of the main loop. Increase if decoding is not error-free.

     """
        
    m,n=H.shape
    if not len(y)==n:
        raise ValueError('Size of y must be equal to number of parity matrix\'s columns n')

    if m>=n:
        raise ValueError('H must be of shape (m,n) with m < n')

    sigma = 10**(-SNR/20)
    
    p0 = np.zeros(shape=n)
    p0 = f1(y,sigma)/(f1(y,sigma) + fM1(y,sigma))
    p1 = np.zeros(shape=n)
    p1 = fM1(y,sigma)/(f1(y,sigma) + fM1(y,sigma))


    #### ETAPE 0 : Initialization 
    q0 = np.zeros(shape=(m,n))
    q0[:] = p0

    q1 = np.zeros(shape=(m,n))
    q1[:] = p1

    r0 = np.zeros(shape=(m,n))
    r1 = np.zeros(shape=(m,n))

    count=0
    prod = np.prod
    
    Bits,Nodes = BitsAndNodes(H)

    
    while(True):
        count+=1

        #### ETAPE 1 : Horizontale

        deltaq = q0 - q1
        deltar = r0 - r1 
        

        for i in range(m):

            Ni=Bits[i]
            for j in Ni:

                Nij = copy.copy(Ni)

                if j in Nij: Nij.remove(j)
                deltar[i,j] = prod(deltaq[i,Nij])
                

        r0 = 0.5*(1+deltar)
        r1 = 0.5*(1-deltar)


        #### ETAPE 2 : Verticale

        for j in range(n):
            Mj = Nodes[j]
            for i in Mj:
                Mji = copy.copy(Mj)
                if i in Mji: Mji.remove(i)
                    
                q0[i,j] = p0[j]*prod(r0[Mji,j])
                q1[i,j] = p1[j]*prod(r1[Mji,j])
                
                if q0[i,j] + q1[i,j]==0:
                    q0[i,j]=0.5
                    q1[i,j]=0.5
              
                else:
                    alpha=1/(q0[i,j]+q1[i,j]) #Constante de normalisation alpha[i,j] 

                    q0[i,j]*= alpha
                    q1[i,j]*= alpha # Maintenant q0[i,j] + q1[i,j] = 1


        q0_post = np.zeros(n)
        q1_post = np.zeros(n)
        
        for j in range(n):
            Mj=Nodes[j]
            q0_post[j] = p0[j]*prod(r0[Mj,j])
            q1_post[j] = p1[j]*prod(r1[Mj,j])
            
            if q0_post[j] + q1_post[j]==0:
                q0_post[j]=0.5
                q1_post[j]=0.5
                
            alpha = 1/(q0_post[j]+q1_post[j])
            
            q0_post[j]*= alpha
            q1_post[j]*= alpha
        

        x = np.array(q1_post > q0_post).astype(int)
        
        if pyldpc.InCode(H,x) or count >= max_iter:  
            break
  
    return x


