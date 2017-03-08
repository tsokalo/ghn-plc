import numpy as np
import scipy
from .soundformat import Audio2Bin, Bin2Audio
from .codingfunctions import Coding
from .decodingfunctions import Decoding_BP_ext, Decoding_logBP_ext, DecodedMessage
from .ldpcalgebra import BitsAndNodes
from math import ceil
import warnings

__all__=['Audio2Bin','Bin2Audio','SoundCoding','SoundDecoding','BER_audio']

def SoundCoding(tG,audio_bin,snr):
    
    """ 
    
    Codes a binary audio array (Therefore must be a 2D-array shaped (length,17)). It is reshaped so as to match tG's dimensions. 
    Then a gaussian noise N(0,snr) is added to the codeword.
    
    Remember SNR: Signal-Noise Ratio: SNR = 10log(1/variance) in decibels of the AWGN used in coding.

    Of course, "listening" to an audio file with n-bits array is impossible, that's why if Coding Matrix G is systematic,
    reading the noisy sound can be possible by gathering the 17 first bits of each 
    n-bits codeword to the left, the redundant bits are dropped. 
    
    returns  a tuple: the (X,n) coded audio, and the noisy one in binary form (length).
    
    Parameters:

    tG: Transposed Coding Matrix G - must be systematic. See CodingMatrix_systematic.
    audio_bin: 2D-array of a binary audio shaped (length,17).
    SNR: Signal-Noise Ratio, SNR = 10log(1/variance) in decibels of the AWGN used in coding.
    
    
    Returns:
    Tuple: binary noisy_audio, coded_audio
    
    
    """
    
    n,k = tG.shape
    length = audio_bin.shape[0]
    
    ratio = (length*17)//k
    rows = ceil(length*17/k)
    rest = (length*17)%k
        
    audio_bin_reshaped = np.zeros((rows,k),dtype=int)

    audio_bin_reshaped[:ratio,:k] = audio_bin.flatten()[:ratio*k].reshape(ratio,k)
    
    if rest >0:
        audio_bin_reshaped[-1,:rest] = audio_bin.flatten()[-rest:]
        
    if n>=100 and not type(tG)==scipy.sparse.csr_matrix:
        warnings.warn("Using scipy.sparse.csr_matrix format is highly recommanded when computing coding and decoding with large matrices to speed up calculations.")
        
    if n>=100 and not (tG[:k,:]==np.identity(k)).all():
        raise ValueError("G must be Systematic if n>=100 (for later decoding, solving tG.tv = tx for images has a O(n^3) complexity.)")
       

    coded_audio = np.zeros(shape=(rows+1,n))
    coded_audio[-1,0]=length
    
    
    for i in range(rows):
        coded_number = Coding(tG,audio_bin_reshaped[i,:],snr)
        coded_audio[i,:] = coded_number
                
    noisy_audio = (coded_audio[:,:k]<0).astype(int).flatten()[:length*17].reshape(length,17)
    
    return coded_audio,noisy_audio
    
    
    
    
def SoundDecoding(tG,H,audio_coded,snr,max_iter=1,log=1):
    
    """ 
    Sound Decoding Function. Taked the 2-D binary coded audio array where each element is a codeword n-bits array and decodes 
    every one of them. Needs H to decode and G to solve v.G = x where x is the codeword element decoded by the function
    itself. When v is found for each codeword, the decoded audio is returned in binary form. It can then be compared to audio_bin 
    with BER_audio function.
    
    Parameters: 
    
    tG: Transposed Coding Matrix must be systematic.
    H: Parity-Check Matrix (Decoding matrix). 
    audio_coded: binary coded audio returned by the function SoundCoding. Must be shaped (length, n) where n is a
                the length of a codeword (also the number of H's columns)
    
    snr: Signal-Noise Ratio: SNR = 10log(1/variance) in decibels of the AWGN used in coding.
    
    log: (optional, default = True), if True, Full-log version of BP algorithm is used. 
    max_iter: (optional, default =1), number of iterations of decoding. increase if snr is < 5db. 

    
    """
    
    n,k = tG.shape
    rows,N = audio_coded.shape
    length = int(audio_coded[-1,0])
    
    if N!=n:
        raise ValueError('Coded Image must have the same number of columns as H')
        
    if n>=100 and not type(tG)==scipy.sparse.csr_matrix:
        warnings.warn("Using scipy.sparse.csr_matrix format is highly recommanded when computing coding and decoding with large matrices to speed up calculations.")
        
    if n>=100 and not (tG[:k,:]==np.identity(k)).all():
        raise ValueError("G must be Systematic. Solving tG.tv = tx for images has a O(n^3) complexity.")
            
    audio_decoded = np.zeros(shape=(rows-1,k),dtype = int)

    if log:
        DecodingFunction = Decoding_logBP_ext
    else:
        DecodingFunction = Decoding_BP_ext
       
    BitsNodes = BitsAndNodes(H)

    for j in range(rows-1):

        decoded = DecodingFunction(H,BitsNodes,audio_coded[j,:],snr,max_iter)

        audio_decoded[j,:] = decoded[:k]
    
    audio_decoded = audio_decoded.flatten()[:length*17].reshape(length,17)
 
    return audio_decoded
    
def BER_audio(original_audio_bin,decoded_audio_bin):
    """ 
    
    Computes Bit-Error-Rate (BER) by comparing 2 binary audio arrays.
    The ratio of bit errors over total number of bits is returned.
    
    """
    if not original_audio_bin.shape == decoded_audio_bin.shape:
        raise ValueError('Original and decoded audio files\' shapes don\'t match !')
        
    length, k = original_audio_bin.shape 
    
    total_bits  = np.prod(original_audio_bin.shape)

    errors_bits = sum(abs(original_audio_bin-decoded_audio_bin).reshape(length*k))
    
    BER = errors_bits/total_bits 
    
    return(BER)
