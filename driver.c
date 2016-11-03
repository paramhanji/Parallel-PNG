/*
  g++ blah.c -lOpenCL
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <CL/opencl.h>



unsigned long long int n;
const unsigned long long int N= 22801763515LL;
const unsigned long long int rootN = 151019; // must be -1 mod 30, 151002+17
const unsigned int piRootN = 13933;
unsigned int *primesToRootN;
unsigned char *primesToRootNmod30;



long long int begin, end;
unsigned int blockSize = 1024*1024*100; // must be 0 mod 30
unsigned int sieveBytes = blockSize;// / 30 + 1;
unsigned char *sieve;


char *kernelSrc;


/*
** loadKernel **
  Parameters: fileName - string that is a filename
  Return:     1 for success, 0 for failure.
  Side-Effect: set global kernelSrc to a string that has the contents of 
               the file.  It must be free-ed at some point.
 */
int loadKernel(char *fileName) {
  FILE * f = fopen(fileName, "rb");
  if (f == NULL) return 0;

  // how many bytes are there in the file.
  if (fseek(f, 0, SEEK_END)) return 0;
  int numBytes = ftell(f);

  // allocate enough memory
  kernelSrc = (char *)malloc(numBytes+1);

  // read the file.
  if (fseek(f, 0, SEEK_SET)) return 0;
  if (fread(kernelSrc, 1, numBytes, f) != numBytes) return 0;
  kernelSrc[numBytes] = '\0';
  if (fclose(f)) return 0;

  return 1;
}

// check the return value retVal from an OpenCL function call.  If there 
// is an error print the error value.
void CLcall(cl_int retVal) {
  if (retVal == CL_SUCCESS) return;

  printf("Error on OpenCL call: code %x in hex, %d integer\n", retVal, retVal);
}


// callback function sometimes used in OpenCL function calls.  If there is an 
// error, this function gets called - so just print the error.
void clError (const char *errinfo, 
	       const void *private_info, 
	       size_t cb, 
	       void *user_data) {
  printf("OpenCL error: %s\n", errinfo);
}

cl_platform_id platforms[10]; // platforms on this computer, typically one
                              // per OpenCL SDK installed.  often only one.
                              // there is only one installed on CS.
cl_device_id device; // a handle/reference to a particular CPU/GPU device.
cl_context context;  // for a particular device, context can be thought of 
                     // as a container to contain all the transactions, 
                     // etc. for communicating with and using the device.
cl_command_queue queue; // think of it as a job queue for the context, with 
                        // commands like - transfer from host to GPU memory,
                        // run a kernel program on the GPU, transfer from 
                        // GPU memory to host memory, etc.
cl_program program;  // a program to run on the GPU, built from a kernel file.
cl_kernel kernel;    // a particular instantiation of a program, ready to 
                     // run on the GPU.
cl_mem clPrimes, clSieve; // these are memory objects, for dealing with 
                          // memory on the GPU.

void initOpenCL() {
  // read in the kernel file...
  char fileName[] = "primesKernel.cl";
  if (!loadKernel(fileName)) {
    printf("Unable to load kernel file.\n");
    return;
  }

  cl_int err; // for storing error code.

  // Connect to a compute device, create a context and a command queue
  cl_uint num;
  CLcall(clGetPlatformIDs(10,platforms,&num));
  CLcall(clGetDeviceIDs(platforms[0],CL_DEVICE_TYPE_GPU, 1, &device, &num));
  context = clCreateContext(0, 1, & device, clError, NULL, NULL);
  queue = clCreateCommandQueue(context, device, 0, &err);
  CLcall(err);
  
  // Create and build a program from our OpenCL-C source code
  program = clCreateProgramWithSource(context, 1, 
						 (const char **) &kernelSrc, 
						 NULL, &err);
  CLcall(err);

  // create a program and load the kernel C file into it.
  CLcall(clBuildProgram(program, 0, NULL, NULL, NULL, NULL));
  cl_build_status status;
  CLcall(clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_STATUS,
			       sizeof(cl_build_status), 
			       &status, NULL));
  if (status != CL_BUILD_SUCCESS) {
    char msg[1001];
    CLcall(clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
				 sizeof(char)*1000, msg, NULL));
    printf("Build error.  Build log is ...\n%s\n",msg);
  }

  // Create a kernel from our program
  kernel = clCreateKernel(program, "primes", &err);
  CLcall(err);

  // Allocate input and output buffers.
  clPrimes = clCreateBuffer(context,  CL_MEM_READ_ONLY,  
			    sizeof(unsigned int) * piRootN, 
			    NULL, &err);
  CLcall(err);
  clSieve = clCreateBuffer(context, CL_MEM_READ_WRITE, 
			   sieveBytes, 
			   NULL, &err);
  CLcall(err);
}


/*
** clSievePrimes **
Parameters: begin - smallest number to test
            end - largest number to test
	    sieve - memory to store the sieving in
	    sieveBytes - redundant mostly, it normally should be end-begin
Return: void
Side-effect: sieve will be set appropriately by sieving begin to end using
             the primes in the primesToRootN array.  The sieve is computed
	     using the GPU.
 */
void clSievePrimes(long long int begin, long long int end, 
		   unsigned char *sieve, unsigned int sieveBytes) {
  // initialize sieve bytes to be all 1.
  unsigned int i;
  for(i=0;i<sieveBytes;i++) sieve[i] = 1;

  // Copy the sieve array into the GPU.
  CLcall(clEnqueueWriteBuffer(queue, clSieve, CL_TRUE, 0, 
			      sizeof(unsigned char) * sieveBytes, 
			      sieve, 0, 
			      NULL, NULL));

  // Copy the pre-computed primes array into the GPU.
  CLcall(clEnqueueWriteBuffer(queue, clPrimes, CL_TRUE, 0, 
			      sizeof(unsigned int) * piRootN, 
			      primesToRootN, 0, 
			      NULL, NULL));

  // Get the maximum number of work items supported 
  // for this kernel on this device
  size_t global = 0; size_t local = 0;
  CLcall(clGetKernelWorkGroupInfo(kernel, device, CL_KERNEL_WORK_GROUP_SIZE, 
				  sizeof(size_t), 
				  &local, NULL));
  //printf("max local workgroup size = %d\n", local);

  local /= 32;
  global = local * 32;

  // Set the arguments to our kernel, and enqueue it for execution
  // note: the order and type here has to match up with that in the kernel
  //       C file.
  CLcall(clSetKernelArg(kernel, 0, sizeof(cl_mem), &clPrimes));
  CLcall(clSetKernelArg(kernel, 1, sizeof(unsigned int), &piRootN));
  CLcall(clSetKernelArg(kernel, 2, sizeof(cl_mem), &clSieve));
  CLcall(clSetKernelArg(kernel, 3, sizeof(long long int), &begin));
  CLcall(clSetKernelArg(kernel, 4, sizeof(long long int), &end));
  CLcall(clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global, 
				&local, 0, NULL, NULL));

  // Force the command queue to get processed, 
  // wait until all commands are complete
  CLcall(clFinish(queue));
    
  // Read back the results
  CLcall(clEnqueueReadBuffer(queue, clSieve, CL_TRUE, 0, 
			     sizeof(unsigned char) * sieveBytes, 
			     sieve, 0, NULL, NULL ));  
}






int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: ./a.out n\n");
    return 0;
  }

  // initialize openCL stuff...
  initOpenCL();


  // parameters...
  n = atoll(argv[1]);

  // first compute all the primes up to sqrtN, those are the only ones
  // we'll need to do the rest of the sieving.
  // piRootN is the number of primes at most sqrtN.  I multiplied by 2 on the 
  // size of the array because I was going 1 past the end of the array I think.
  primesToRootN = (unsigned int *) malloc(sizeof(unsigned int) * (piRootN*2));
  primesToRootNmod30 = (unsigned char *)malloc(sizeof(unsigned char) * (2*piRootN));
  long long int i, j, k, l, x;

  // primes up to root(N).  sieve to get them.
  unsigned char *sieveRootN = (unsigned char *)
    malloc(sizeof(unsigned char) * (rootN+1));

  // first set everything but 0 and 1 to 1 for "maybe prime".
  for(i=0; i<=rootN; i++) {
    sieveRootN[i] = 1;
  }
  sieveRootN[0] = sieveRootN[1] = 0;

  // now for each prime found, set all it's multiples to be 0 for "composite".
  for(i=2; i*i <= rootN; i++) {
    if (sieveRootN[i]) {
      for(j=i*i; j <= rootN; j+=i) 
	sieveRootN[j] = 0;
    }
  }

  // now we have all the primes, just put them into our primesToRootN array.
  long long int primeCount=0;
  for(i=0; i <=rootN; i++)  {
    if (sieveRootN[i]) {
      primesToRootN[primeCount++] = i;
      primesToRootNmod30[primeCount] = i % 30;
      if (primeCount == n) {
	printf("%lld\n", i);
	return 0;
      }
    }
  }


  // now we have the small primes, time to sieve for the rest...
  sieve = (unsigned char *) 
    malloc(sizeof(unsigned char) * sieveBytes);

  
  begin = rootN+1;

  // so begin is the start of the interval we want to sieve.
  // we'll sieve blockSize at a time until we find the n-th prime.

  end = begin+blockSize;    // sieve blockSize at a time.
  for(;end < N; begin+=blockSize) {
    end = begin+blockSize;    // sieve blockSize at a time.

    // make the GPU do the sieving for us.  when this is done, the sieve
    // will be 1's and 0's, and correct.
    clSievePrimes(begin, end, sieve, sieveBytes);

    // now check what we've got. go through and count how many primes
    // we found.
    int c=0;
    for(i=0;i<sieveBytes;i++) {
      if (sieve[i]) {
	c++;
	x = begin+i;
	if (x < end) {
	  primeCount++;
	  if (primeCount == n) {
	    printf("%lld\n", x);
	    return 0;
	  }
	}
      }
    }
  }


  printf("Number too big... primeCount = %lld\n", primeCount);


  free(kernelSrc);
  free(sieve);
  free(sieveRootN);
  free(primesToRootN);
  free(primesToRootNmod30);
  return 0;
}
