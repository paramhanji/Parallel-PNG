import numpy as np
from math import exp 
from libc.math cimport exp as c_exp
from cython.parallel import prange, parallel
cimport openmp

def encrypt(exp, mod, plaintext):
	cdef long[:] temp=np.asarray(plaintext)
	cdef int n=len(temp)
	cdef int exponent=exp
	cdef int modulo=mod
	cdef int char
	cdef int res
	cdef int tempExponent
	cdef int i
	cdef long[:] cipher=np.zeros(len(temp), dtype=long)
	with nogil, parallel():
		for i in prange(n):
			char=temp[i]
			tempExponent=exponent
			res=1
			char=char%modulo
			while tempExponent>0:
				if tempExponent&1:
					res=(res*char)%modulo
				tempExponent=tempExponent>>1
				char=(char*char)%modulo
			#print res
			cipher[i]=res
	return cipher
	
	
"""
int power(int x, unsigned int y, int p)
{
    int res = 1;      // Initialize result
 
    x = x % p;  // Update x if it is more than or 
                // equal to p
 
    while (y > 0)
    {
        // If y is odd, multiply x with result
        if (y & 1)
            res = (res*x) % p;
 
        // y must be even now
        y = y>>1; // y = y/2
        x = (x*x) % p;  
    }
    return res;
}
"""
