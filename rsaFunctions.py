import random


def egcd(a, b):
	if a == 0:
		return (b, 0, 1)
	else:
		g, y, x = egcd(b % a, a)
	return (g, x - (b // a) * y, y)


def getMultiplicativeInverse(a, m):
	g, x, y = egcd(a, m)
	if g != 1:
		raise Exception('modular inverse does not exist')
	else:
		return x % m


def isPrime(n):
	if n == 2 or n == 3: 
		return True
	if n < 2 or n%2 == 0: 
		return False
	if n < 9: 
		return True
	if n%3 == 0: 
		return False
	sqrt = int(n**0.5)
	f = 5
	while f <= sqrt:
		#print '\t',f
		if n%f == 0:
			return False
		if n%(f+2) == 0:
			return False
		f +=6
	return True  


def getKeyPair(p, q):
	if not (isPrime(p) and isPrime(q) and p!=q):
		print "Error in p/q"
	n = p * q
	t = (p-1) * (q-1)
	e = random.randrange(1, t)
	g, temp1, temp2 = egcd(e, t)
	while g != 1:
		e = random.randrange(1, t)
		g, temp1, temp2 = egcd(e, t)
	d = getMultiplicativeInverse(e, t)
	return ((e, n), (d, n))


def encrypt(key, plaintext):
	exponent, modulo = key
	encrypt=[]
	for char in plaintext:
		encrypt.append(pow(ord(char),exponent,modulo))
	#print encrypt
	return encrypt


def decrypt(key, ciphertext):
	exponent, modulo = key
	decrypt=[]
	for char in ciphertext:
		decrypt.append(chr(pow(char, exponent, modulo)))
	#print decrypt
	return decrypt
