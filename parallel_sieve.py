import numpy as np
import pyopencl as cl
import array

piRootN = 13933
rootN = 151019
blockSize = 1024*1024*100
primesToRootN = np.asarray([0] * (piRootN * 2))

def loadKernel(filename):
	kernelSrc = ''
	f = open(filename, 'r')
	kernelSrc = "".join(f.readlines())
	'''with open(filename, 'rb') as f:
		for data in f:
			kernelSrc += data'''

	return kernelSrc


def initOpenCL():
	filename = 'sieve.cl'
	kernelSrc=loadKernel(filename)
	if not kernelSrc:
		print "Unable to load kernel"
		return

	context = cl.create_some_context()
	queue = cl.CommandQueue(context)

	program = cl.Program(context, kernelSrc).build()
	return context, program, queue


def clSievePrimes(begin, end):# context, program, queue):
	
	filename = 'sieve.cl'
	kernelSrc=loadKernel(filename)
	if not kernelSrc:
		print "Unable to load kernel"
		return

	context = cl.create_some_context()
	queue = cl.CommandQueue(context)

	program = cl.Program(context, kernelSrc).build()
	#return context, program, queue

	mf = cl.mem_flags
	sieve = np.asarray([0] * blockSize)

	clSieve = cl.Buffer(context, mf.WRITE_ONLY, sieve.nbytes)
	clPrimes = cl.Buffer(context, mf.READ_ONLY | mf.COPY_HOST_PTR, hostbuf=primesToRootN)
	program.primes(queue, np.random.rand(1024).shape, None, clPrimes, np.int32(piRootN), clSieve, np.int64(begin), np.int64(end))
	return_sieve = np.empty_like(sieve)
	cl.enqueue_read_buffer(queue, clSieve, return_sieve)
	return return_sieve



n = input("Enter n - ")
#context, program, queue = initOpenCL()

maybePrime = [True] * (rootN + 1)
maybePrime[0] = False
maybePrime[1] = False

i = 2
while(i*i <= rootN):
	if maybePrime[i]:
		j = i*i
		while(j <= rootN):
			#print j
			maybePrime[j] = False
			j += i
	i += 1

primeCount = 0
for i in range(rootN+1):
	if maybePrime[i]:
		primesToRootN[primeCount] = i
		primeCount += 1

		if primeCount == n:
			print n, "th prime is - ", i
			exit(0)

begin = rootN + 1
end = begin+blockSize

while(True):
	end = begin+blockSize
	sieve = clSievePrimes(begin, end)#, context, program, queue)
	
	for i in range(blockSize):
		if sieve[i] == 1:
			x = begin+i
			if x<end:
				primeCount += 1
				print n, "th prime is - ", x
				exit(0)

	begin += blockSize