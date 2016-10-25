import numpy as np
from math import exp 
from libc.math cimport exp as c_exp
from cython.parallel import prange, parallel
cimport openmp

def encrypt(exp, mod, plaintext):
	cdef long[:] temp=np.asarray(plaintext)
	cdef unsigned long long n=len(temp)
	cdef unsigned long long exponent=exp
	cdef unsigned long long modulo=mod
	cdef unsigned long long c
	cdef unsigned long long res=0
	cdef unsigned long long tempExponent=0
	cdef unsigned long int i=0
	cdef long long[:] cipher=np.zeros(len(temp), dtype=long)
	with nogil, parallel():
		for i in prange(n):
			c=temp[i]
			tempExponent=exponent
			res=1
			c=c%modulo
			while tempExponent>0:
				if tempExponent&1:
					res=(res*c)%modulo
				tempExponent=tempExponent>>1
				c=(c*c)%modulo
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
