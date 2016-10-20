import rsaFunctions
from encrypt import encrypt
from decrypt import decrypt

if __name__ == '__main__':
	p = raw_input("Enter p : ")
	q = raw_input("Enter q : ")
	p=int(p)
	q=int(q)
	LLI = (p*q)>(2**15-1) # Check if product of p,q is higher than long long int
	print "Is long: ",LLI
	public, private = rsaFunctions.getKeyPair(p, q)
	print "Public key : ", public ," Private key : ", private
	msg = raw_input("Enter message : ")
	if LLI:
		encMsg = rsaFunctions.encrypt((public[0], public[1]), msg)
	else:
		msgInt = map(lambda x:ord(x),msg)
		encMsg = encrypt(public[0], public[1], msgInt)
	print "Encrypted message : "
	for i in range(len(encMsg)):
		print encMsg[i],
	if LLI:
		dec = rsaFunctions.decrypt((private[0], private[1]), encMsg)
	else:
		decMsg = decrypt(private[0], private[1], encMsg)
		dec = map(lambda x:chr(x),decMsg)
	print "\nDecrypted Message :"
	print dec
