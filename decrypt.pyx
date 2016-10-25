import numpy as np
from math import exp 
from libc.math cimport exp as c_exp
from cython.parallel import prange, parallel
cimport openmp

def decrypt(exp, mod, ciphertext):
	cdef long long[:] temp=np.asarray(ciphertext)
	cdef unsigned long long n=len(temp)
	cdef unsigned long long exponent=exp
	cdef unsigned long long modulo=mod
	cdef unsigned long long c=0
	cdef unsigned long long res=0
	cdef unsigned long long tempExponent=0
	cdef unsigned long long i=0
	cdef long long[:] plaintext=np.zeros(len(temp), dtype=long)
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
			plaintext[i]=res
	return plaintext
