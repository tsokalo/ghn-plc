import numpy as np
from .imagesformat import int2bitarray, bitarray2int, Bin2Gray, Gray2Bin, RGB2Bin, Bin2RGB
from .codingfunctions import Coding
from .decodingfunctions import Decoding_BP_ext, Decoding_logBP_ext, DecodedMessage
from .ldpcalgebra import Bits2i, Nodes2j, BitsAndNodes
import scipy
import warnings
from math import ceil

__all__=['ImageCoding','ImageDecoding','int2bitarray','bitarray2int','Bin2Gray',
		'Gray2Bin','RGB2Bin','Bin2RGB','BER']
		
		
def ImageCoding(tG,img_bin,snr):
    
    """ 
    
    CAUTION: SINCE V.0.7 Image coding and decoding functions TAKE TRANSPOSED CODING MATRIX tG. USE
    SCIPY.SPARSE.CSR_MATRIX FORMAT (IN H AND G) TO SPEED UP CALCULATIONS. USE A SYSTEMATIC CODING MATRIX WITH 
    CodingMatrix_systematic. THEN USE SCIPY.SPARSE.CSR_MATRIX()
    
    --------
    
    Codes a binary image (Therefore must be a 3D-array). The image is reshaped so as to match the Coding and decoding matrices.
    
    - The last line of the coded image stores the initial image's shape so as to be able to construct the decoded image again 
    via a reshape. 
    - n is the length of a codeword. Then a gaussian noise N(0,snr) is added to the codeword.
    
    Remember SNR: Signal-Noise Ratio: SNR = 10log(1/variance) in decibels of the AWGN used in coding.
 
    ImageCoding returns  a tuple: the reshaped coded image, and the noisy (binary) image.
    
    Parameters:

    tG: Transposed Coding Matrix G - must be systematic. See CodingMatrix_systematic.
    img_bin: 3D-array of a binary image.
    SNR: Signal-Noise Ratio, SNR = 10log(1/variance) in decibels of the AWGN used in coding.    
    
    Returns:
    (default): Tuple:  coded_img, noisy_img
    
    """
    n,k = tG.shape
    height,width,depth = img_bin.shape
    
    if n>=100 and not type(tG)==scipy.sparse.csr_matrix:
        warnings.warn("Using scipy.sparse.csr_matrix format is highly recommanded when computing coding and decoding with large matrices to speed up calculations.")
        
    if n>=100 and not (tG[:k,:]==np.identity(k)).all():
        raise ValueError("G must be Systematic if n>=100 (for later decoding, solving tG.tv = tx for images has a O(n^3) complexity.)")
       

    ratio = height*width*depth//k
    rows = ceil(height*width*depth/k)
    rest = height*width*depth%k
        
    img_bin_reshaped = np.zeros((rows,k),dtype=int)
    
    img_bin_reshaped[:ratio,:] = img_bin.flatten()[:ratio*k].reshape(ratio,k)
    if rest >0:
        img_bin_reshaped[-1,:rest] = img_bin.flatten()[-rest:]

    coded_img = np.zeros(shape=(rows+1,n))
    coded_img[rows,0:3]=height,width,depth
    
    for i in range(rows):
        coded_img[i,:] = Coding(tG,img_bin_reshaped[i,:],snr)
        
    noisy_img = (coded_img[:rows,:k]<0).astype(int).flatten()[:height*width*depth].reshape(height,width,depth)

    
    return coded_img,noisy_img


def ImageDecoding(tG,H,img_coded,snr,max_iter=1,log=1):
    
    """ 
        
    CAUTION: SINCE V.0.7 ImageDecoding TAKES TRANSPOSED CODING MATRIX tG INSTEAD OF G.IF N>100, USE SCIPY.SPARSE.CSR_MATRIX 
    FORMAT (IN H AND G) TO SPEED UP CALCULATIONS. 
    
    --------
    Image Decoding Function. Taked the 3-D binary coded image where each row is a codeword n-bits array and decodes 
    every one of them. Needs H to decode and tG to solve tG.tv = tx where x is the codeword element decoded by the function
    itself. When v is found for each codeword, the decoded image is returned in a binary form. 
    
    Parameters: 
    
    tG: Transposed Coding Matrix ( IF N>100, SCIPY.SPARSE.CSR_MATRIX FORMAT RECOMMANDED )
    H: Parity-Check Matrix (Decoding matrix).( IF N>100, SCIPY.SPARSE.CSR_MATRIX FORMAT RECOMMANDED)

    img_coded: binary coded image returned by the function ImageCoding.
    
    snr: Signal-Noise Ratio: SNR = 10log(1/variance) in decibels of the AWGN used in coding.
    
    max_iter: (optional, default =1), number of iterations of decoding. 
    
    log: (optional, default = True), if True, Full-log version of BP algorithm is used. 

    
    """
    
    n,k = tG.shape
    size = img_coded[-1,0:3].astype(int)
    img_coded = img_coded[:-1,:]
    rows,N = img_coded.shape
    depth = size[-1]
    
    if N!=n:
        raise ValueError('Coded Image must have the same number of columns as H')
        
    if depth !=8 and depth != 24:
        raise ValueError('type of image not recognized: third dimension of the binary image must be 8 for grayscale, or 24 for RGB images')

    if n>=100 and not type(tG)==scipy.sparse.csr_matrix:
        warnings.warn("Using scipy.sparse.csr_matrix format is highly recommanded when computing coding and decoding with large matrices to speed up calculations.")
        
    if n>=100 and not (tG[:k,:]==np.identity(k)).all():
        raise ValueError("G must be Systematic. Solving tG.tv = tx for images has a O(n^3) complexity.")
        
    img_decoded_bin = np.zeros(shape=(rows,k),dtype = int)

    if log:
        DecodingFunction = Decoding_logBP_ext
    else:
        DecodingFunction = Decoding_BP_ext
    
    BitsNodes = BitsAndNodes(H)

    for i in range(rows):
        decoded_vector = DecodingFunction(H,BitsNodes,img_coded[i,:],snr,max_iter)
        img_decoded_bin[i,:] = decoded_vector[:k]
            

        img_decoded = img_decoded_bin.flatten()[:np.prod(size)].reshape(size)
 
    return img_decoded

def BER(original_img_bin,decoded_img_bin):
    """ 
    
    Computes Bit-Error-Rate (BER) by comparing 2 binary images.
    The ratio of bit errors over total number of bits is returned.
    
    """
    if not original_img_bin.shape == decoded_img_bin.shape:
        raise ValueError('Original and decoded images\' shapes don\'t match !')
        
    height, width, k = original_img_bin.shape 
    
    
    errors_bits = sum(abs(original_img_bin-decoded_img_bin).reshape(height*width*k))
    total_bits  = np.prod(original_img_bin.shape)
    
    BER = errors_bits/total_bits 
    
    return(BER)


