__kernel void primes( __global uint* primes, __private uint numPrimes,
					  __global uint* sieve,
					  __private ulong begin, __private ulong end)
{
	uint id = get_global_id(0);
	uint numTasks = get_global_size(0);
	  
	uint i, x;
	ulong j;

	ulong mybegin, myend;
	mybegin = begin + id*(end-begin)/numTasks;
	myend = begin + (id+1)*(end-begin)/numTasks;

	for(i=0;i<numPrimes;i++) {
		x = primes[i];

		j = (begin / x) * x;
		if (j < begin)
			j += x;

		j += id*x;
		    
		for(;j<end;j += numTasks*x)
		sieve[j-begin] = 0;
	}
} 
