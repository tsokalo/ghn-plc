#!/usr/bin/python

import pyldpc
import numpy as np

from ghn_ldpc_matrix import *

def EncodingBPSK(tG,v,SNR):
	
    """
    
    IMPORTANT: if H is large, tG can be transposed coding matrix scipy.sparse.csr_matrix object to speed up calculations.

    Codes a message v with Coding Matrix G, and sends it through a noisy (default)
    channel. 
    
    G's shape is (k,n). 

    Message v is passed to tG: d = tG.tv d is a n-vector turned into a BPSK modulated
    vector x. Then Additive White Gaussian Noise (AWGN) is added. 
    
    SNR is the Signal-Noise Ratio: SNR = 10log(1/variance) in decibels, where variance is the variance of the AWGN.
    Remember: 
    
        1. d = v.G (or (td = tG.tv))
        2. x = BPSK(d) (or if you prefer the math: x = pow(-1,d) )
        3. y = x + AWGN(0,snr) 

    
    Parameters:

    tG: 2D-Array (OR scipy.sparse.csr_matrix)Transposed Coding Matrix obtained from CodingMatrix functions.
    v: 1D-Array, k-vector (binary of course ..) 
    SNR: Signal-Noise-Ratio: SNR = 10log(1/variance) in decibels. 

    -------------------------------

    Returns y

    """
    n,k = tG.shape

    if len(v)!= k:
        raise ValueError(" Size of message v must be equal to number of Coding Matrix G's rows " )  
    
    d = pyldpc.BinaryProduct(tG,v)
    x=pow(-1,d)

    sigma = 10**(-SNR/20)
    e = np.random.normal(0,sigma,size=n)


    y=x+e

    return y

def CodingBPSK(tG,v,SNR):
	
    """
    
    IMPORTANT: if H is large, tG can be transposed coding matrix scipy.sparse.csr_matrix object to speed up calculations.

    Codes a message v with Coding Matrix G, and sends it through a noisy (default)
    channel. 
    
    G's shape is (k,n). 

    Message v is passed to tG: d = tG.tv d is a n-vector turned into a BPSK modulated
    vector x. Then Additive White Gaussian Noise (AWGN) is added. 
    
    SNR is the Signal-Noise Ratio: SNR = 10log(1/variance) in decibels, where variance is the variance of the AWGN.
    Remember: 
    
        1. d = v.G (or (td = tG.tv))
        2. x = BPSK(d) (or if you prefer the math: x = pow(-1,d) )
        3. y = x + AWGN(0,snr) 

    
    Parameters:

    tG: 2D-Array (OR scipy.sparse.csr_matrix)Transposed Coding Matrix obtained from CodingMatrix functions.
    v: 1D-Array, k-vector (binary of course ..) 
    SNR: Signal-Noise-Ratio: SNR = 10log(1/variance) in decibels. 

    -------------------------------

    Returns y

    """
    n,k = tG.shape

    if len(v)!= k:
        raise ValueError(" Size of message v must be equal to number of Coding Matrix G's rows " )  
    
    d = pyldpc.BinaryProduct(tG,v)
    x=pow(-1,d)

    sigma = 10**(-SNR/20)
    e = np.random.normal(0,sigma,size=n)


    y=x+e

    return y
