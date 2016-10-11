/*
  g++ blah.c -lOpenCL
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <CL/opencl.h>


/*
	Some hard-coded constants. These need to be optimised.
*/
unsigned long long int n;
const unsigned long long int N= 22801763515LL;
const unsigned long long int rootN = 151019;
const unsigned int piRootN = 13933;
unsigned int *primesToRootN;
unsigned char *primesToRootNmod30;

long long int begin, end;
unsigned int blockSize = 1024*1024*100;
unsigned char *sieve;
char *kernelSrc;


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


cl_platform_id platforms[10]; // platforms on this computer
cl_device_id device; // a handle/reference to a particular CPU/GPU device.
cl_context context;
cl_command_queue queue;
cl_program program;
cl_mem clPrimes, clSieve;


void initOpenCL() {
	// read in the kernel file...
	char fileName[] = "sieve.cl";
	if (!loadKernel(fileName)) {
		printf("Unable to load kernel file.\n");
		return;
	}

	cl_int err; // for storing error code.

	// Connect to a compute device, create a context and a command queue
	cl_uint num;
	clGetPlatformIDs(10,platforms,&num);
	clGetDeviceIDs(platforms[0],CL_DEVICE_TYPE_GPU, 1, &device, &num);
	context = clCreateContext(0, 1, & device, NULL, NULL, NULL);
	queue = clCreateCommandQueue(context, device, 0, &err);
	 
	// Create and build a program from our OpenCL-C source code
	program = clCreateProgramWithSource(context, 1, 
										(const char **) &kernelSrc, 
					 					NULL, &err);

	// create a program and load the kernel C file into it.
	clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	cl_build_status status;
	clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_STATUS,
						  sizeof(cl_build_status),
						  &status, NULL);
	if (status != CL_BUILD_SUCCESS) {
		char msg[1001];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
							  sizeof(char)*1000, msg, NULL);
		printf("Build error.  Build log is ...\n%s\n",msg);
	}

	// Create a kernel from our program
	kernel = clCreateKernel(program, "primes", &err);

	// Allocate input and output buffers.
	clPrimes = clCreateBuffer(context,  CL_MEM_READ_ONLY,  
							  sizeof(unsigned int) * piRootN, 
							  NULL, &err);
	clSieve = clCreateBuffer(context, CL_MEM_READ_WRITE, 
							 blockSize, 
							 NULL, &err);
}


/*
	begin - smallest number to test
	end - largest number to test
	sieve - memory to store the sieving in
 */
void clSievePrimes(long long int begin, long long int end, 
				   unsigned char *sieve) {
		// initialize sieve bytes to be all 1.
		unsigned int i;
		for(i=0;i<blockSize;i++)
			sieve[i] = 1;

	// Copy the sieve array into the GPU.
	clEnqueueWriteBuffer(queue, clSieve, CL_TRUE, 0, 
						 sizeof(unsigned char) * blockSize, 
						 sieve, 0, 
						 NULL, NULL);

	// Copy the pre-computed primes array into the GPU.
	clEnqueueWriteBuffer(queue, clPrimes, CL_TRUE, 0, 
						 sizeof(unsigned int) * piRootN, 
						 primesToRootN, 0, 
						 NULL, NULL);

	// Get the maximum number of work items supported 
	// for this kernel on this device
	size_t global = 0; size_t local = 0;
	clGetKernelWorkGroupInfo(kernel, device, CL_KERNEL_WORK_GROUP_SIZE, 
							 sizeof(size_t), 
							 &local, NULL);
	//printf("max local workgroup size = %d\n", local);
	printf("\n%lu\n", local);
	local /= 32;
	global = local * 32;
	printf("\n%lu\n", local);
	printf("\n%lu\n", global);

	// Set the arguments to our kernel, and enqueue it for execution
	clSetKernelArg(kernel, 0, sizeof(cl_mem), &clPrimes);
	clSetKernelArg(kernel, 1, sizeof(unsigned int), &piRootN);
	clSetKernelArg(kernel, 2, sizeof(cl_mem), &clSieve);
	clSetKernelArg(kernel, 3, sizeof(long long int), &begin);
	clSetKernelArg(kernel, 4, sizeof(long long int), &end);
	clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global,
						   &local, 0, NULL, NULL);

	// Wait until all commands are complete
	clFinish(queue);
	   
	// Read back the results
	clEnqueueReadBuffer(queue, clSieve, CL_TRUE, 0, 
						sizeof(unsigned char) * blockSize, 
						sieve, 0, NULL, NULL );  
}






int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: ./a.out n\n");
		return 0;
	}

	// initialize openCL stuff...
	initOpenCL();

	n = atoll(argv[1]);

	// first compute all the primes up to sqrtN.
	// piRootN is the number of primes at most sqrtN.
	primesToRootN = (unsigned int *) malloc(sizeof(unsigned int) * (piRootN*2));
	primesToRootNmod30 = (unsigned char *)malloc(sizeof(unsigned char) * (2*piRootN));
	long long int i, j, k, l, x;

	// primes up to root(N).  sieve to get them.
	unsigned char *maybePrime = (unsigned char *) malloc(sizeof(unsigned char) * (rootN+1));

	// first set everything but 0 and 1 to 1 for "maybe prime".
	for(i=0; i<=rootN; i++) {
		maybePrime[i] = 1;
	}
	maybePrime[0] = maybePrime[1] = 0;

	// now for each prime found, set all it's multiples to be 0 for "composite".
	for(i=2; i*i <= rootN; i++)
		if (maybePrime[i]) {
			for(j=i*i; j <= rootN; j+=i) 
			maybePrime[j] = 0;
		}

	// now we have all the primes, just put them into our primesToRootN array.
	long long int primeCount=0;
	for(i=0; i <=rootN; i++)  {
		if (maybePrime[i]) {
			primesToRootN[primeCount++] = i;
			primesToRootNmod30[primeCount] = i % 30;
			if (primeCount == n) {
				printf("%lldth prime is %lld\n", n, i);
				return 0;
			}
		}
	}


	// We now have the small primes, time to sieve for the rest.
	sieve = (unsigned char *) malloc(sizeof(unsigned char) * blockSize);
  
	begin = rootN+1;	// start of the interval we want to sieve.

	end = begin+blockSize;    // sieve 1 blockSize at a time.
	for(;end < N; begin+=blockSize) {
		end = begin+blockSize;    // sieve blockSize at a time.

		// Make the GPU do the sieving for us.
		clSievePrimes(begin, end, sieve, blockSize);

		for(i=0;i<blockSize;i++) {
			if (sieve[i]) {
				x = begin+i;
				if (x < end) {
					primeCount++;
					if (primeCount == n) {
						//printf("%lldth prime is %lld\n", n, x);
						//printf("%lld, %lld, %lld\n", begin, i, l);
						free(kernelSrc);
						free(sieve);
						free(maybePrime);
						free(primesToRootN);
						free(primesToRootNmod30);
						return x;
					}
				}
			}
		}
	}


	printf("Number too big... primeCount = %lld\n", primeCount);


	free(kernelSrc);
	free(sieve);
	free(maybePrime);
	free(primesToRootN);
	free(primesToRootNmod30);
	return 0;
}
