from rsaFunctions import getKeyPair
from encrypt import encrypt
from decrypt import decrypt

import subprocess as sp

if __name__ == '__main__':

	# Figure out upper limit for n where a random prime - 1st to nth - is used. 2 such numbers are needed - n1 and n2
	n1 = 10000
	n2 = 20000
	p = sp.call(['./generate_prime ' + str(n1)])
	q = sp.call(['./generate_prime ' + str(n2)])
	
	public, private = getKeyPair(p, q)
	print "Public key : ", public ," Private key : ", private
	msg = raw_input("Enter message : ")
	msgInt = []
	for char in msg:
		msgInt.append(ord(char))
	encMsg = encrypt(public[0], public[1], msgInt)
	print "Encrypted message : "
	for i in range(len(encMsg)):
		print encMsg[i], 
	print "\nDecrypted Message :"
	decMsg = decrypt(private[0], private[1], encMsg)
	dec = []
	for i in range(len(decMsg)):
		dec.append(chr(decMsg[i]))
	print dec
	
