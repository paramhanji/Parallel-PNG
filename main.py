from rsaFunctions import getKeyPair
from encrypt import encrypt
from decrypt import decrypt

if __name__ == '__main__':
	p = raw_input("Enter p : ")
	q = raw_input("Enter q : ")
	p=int(p)
	q=int(q)
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
	
