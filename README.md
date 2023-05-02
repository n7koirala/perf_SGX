
# perf_SGX

Objective of perf_SGX
------------------------
This project aims to evaluate the performance of two operations, namely file I/O with hashing and matrix multiplication, using Intel Software Guard Extensions (SGX) . 

------------------------------------
How to Build/Execute
------------------------------------

1. Install Intel SGX SDK for Linux* using https://github.com/intel/linux-sgx.
2. Make sure your environment is set:
    ```
    $ source ${sgx-sdk-install-path}/environment
    ```
3. Location of files to be sent must be under `/FileIOHash_MaltMat`
4. Change the IP address of the server, port, matrix size and file location in `App.cpp` 
5. Match the IP address and port in `file_client.cpp`
6. Build the project with the prepared Makefile (makes in Hardware Mode, Debug build):
    ```
    $ make
    ```
