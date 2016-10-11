import numpy as np
from math import exp 
from libc.math cimport exp as c_exp
from cython.parallel import prange, parallel
cimport openmp

def decrypt(exp, mod, ciphertext):
	cdef long[:] temp=np.asarray(ciphertext)
	cdef int n=len(temp)
	cdef int exponent=exp
	cdef int modulo=mod
	cdef int char
	cdef int res
	cdef int tempExponent
	cdef int i
	cdef long[:] plaintext=np.zeros(len(temp), dtype=long)
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
			plaintext[i]=res
	return plaintext
