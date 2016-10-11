# Parallel-PNG
Parallelize the popular Sieve of Eratosthenes algorithm using a OpenCL.
Use Prime numbers so generated in implementation of RSA

Requirements:
1) OpenCL (install separately)
2) Numpy (pip install)
3) Cython (pip install)

Code for RSA is implemented in Cython.
Instructions to build code and run:
1) python setupEncrypt.py build_ext --inplace
2) python setupDecrypt.py build_ext --inplace
3) python main.py

Further lines of work:
1) Integer limit workaround

Note:
Functions encrypt/decrypt in rsaFunctions.py work on their own as well, serially implemented in python.
Serial in the sense that each character in the message is encoded serially
There is fast modular multiplication using built in pow() function so parallelisation is implicitdd
