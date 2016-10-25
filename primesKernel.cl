/*
  Kernel function.  This is the source code for the program that will 
  run on the GPU.  It is basically C code, with a slight few changes for 
  special GPU stuff.  

  __kernel specifies the function will be used as a kernel
  entry-point - the function that will be specified in our OpenCL program 
  as the one to load into the GPU.

  __global specifies an array/variable lives in global memory - which is 
  big, but slow.

  __private specifies it lives in private memory - which is small, but fast - 
  like cache.
 */

__kernel void primes( __global uint* primes, __private uint numPrimes,
		      __global uchar* sieve,
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

    /*
    j = (mybegin / x) * x;
    if (j < mybegin) j += x;
    if (j % 2 == 0) j+=x;

    for(;j<myend; j += x)
      sieve[j-begin] = 0;
    */
  
    j = (begin / x) * x;
    if (j < begin) j += x;

    j += id*x;
    
    for(;j<end;j += numTasks*x)
      sieve[j-begin] = 0;
  }
} 
