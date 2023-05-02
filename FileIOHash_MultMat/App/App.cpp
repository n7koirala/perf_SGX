/*
 * Copyright (C) 2011-2018 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

//To compile a non-SGX version, use this command:
// g++ App/App.cpp Enclave/Enclave.cpp -pedantic -Wall  -O3 -o ./app -lm -DNOENCLAVE
//Use the NOENCLAVE macro as appropriate to strip out SGX-specific code.
//See main function for examples on how to modify ECALLs

#include <chrono> //for time

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <cstdint>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>

#include <cstdlib>
#include <ctime>

# include <unistd.h>
# include <pwd.h>
# define MAX_PATH FILENAME_MAX

#ifndef NOENCLAVE

#include "sgx_urts.h"
#include "App.h"
#include "Enclave_u.h"

using namespace std;

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;

#define PORT 8060  // the port users will be connecting to

typedef struct _sgx_errlist_t {
    sgx_status_t err;
    const char *msg;
    const char *sug; /* Suggestion */
} sgx_errlist_t;

/* Error code returned by sgx_create_enclave */
static sgx_errlist_t sgx_errlist[] = {
    {
        SGX_ERROR_UNEXPECTED,
        "Unexpected error occurred.",
        NULL
    },
    {
        SGX_ERROR_INVALID_PARAMETER,
        "Invalid parameter.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_MEMORY,
        "Out of memory.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_LOST,
        "Power transition occurred.",
        "Please refer to the sample \"PowerTransition\" for details."
    },
    {
        SGX_ERROR_INVALID_ENCLAVE,
        "Invalid enclave image.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ENCLAVE_ID,
        "Invalid enclave identification.",
        NULL
    },
    {
        SGX_ERROR_INVALID_SIGNATURE,
        "Invalid enclave signature.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_EPC,
        "Out of EPC memory.",
        NULL
    },
    {
        SGX_ERROR_NO_DEVICE,
        "Invalid SGX device.",
        "Please make sure SGX module is enabled in the BIOS, and install SGX driver afterwards."
    },
    {
        SGX_ERROR_MEMORY_MAP_CONFLICT,
        "Memory map conflicted.",
        NULL
    },
    {
        SGX_ERROR_INVALID_METADATA,
        "Invalid enclave metadata.",
        NULL
    },
    {
        SGX_ERROR_DEVICE_BUSY,
        "SGX device was busy.",
        NULL
    },
    {
        SGX_ERROR_INVALID_VERSION,
        "Enclave version was invalid.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ATTRIBUTE,
        "Enclave was not authorized.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_FILE_ACCESS,
        "Can't open enclave file.",
        NULL
    },
};

/* Check error conditions for loading enclave */
void print_error_message(sgx_status_t ret)
{
    size_t idx = 0;
    size_t ttl = sizeof sgx_errlist/sizeof sgx_errlist[0];

    for (idx = 0; idx < ttl; idx++) {
        if(ret == sgx_errlist[idx].err) {
            if(NULL != sgx_errlist[idx].sug)
                printf("Info: %s\n", sgx_errlist[idx].sug);
            printf("Error: %s\n", sgx_errlist[idx].msg);
            break;
        }
    }

    if (idx == ttl)
    	printf("Error code is 0x%X. Please refer to the \"Intel SGX SDK Developer Reference\" for more details.\n", ret);
}

/* Initialize the enclave:
 *   Step 1: try to retrieve the launch token saved by last transaction
 *   Step 2: call sgx_create_enclave to initialize an enclave instance
 *   Step 3: save the launch token if it is updated
 */
int initialize_enclave(void)
{
    char token_path[MAX_PATH] = {'\0'};
    sgx_launch_token_t token = {0};
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    int updated = 0;

    /* Step 1: try to retrieve the launch token saved by last transaction
     *         if there is no token, then create a new one.
     */
    /* try to get the token saved in $HOME */
    const char *home_dir = getpwuid(getuid())->pw_dir;

    if (home_dir != NULL &&
        (strlen(home_dir)+strlen("/")+sizeof(TOKEN_FILENAME)+1) <= MAX_PATH) {
        /* compose the token path */
        strncpy(token_path, home_dir, strlen(home_dir));
        strncat(token_path, "/", strlen("/"));
        strncat(token_path, TOKEN_FILENAME, sizeof(TOKEN_FILENAME)+1);
    } else {
        /* if token path is too long or $HOME is NULL */
        strncpy(token_path, TOKEN_FILENAME, sizeof(TOKEN_FILENAME));
    }

    FILE *fp = fopen(token_path, "rb");
    if (fp == NULL && (fp = fopen(token_path, "wb")) == NULL) {
        printf("Warning: Failed to create/open the launch token file \"%s\".\n", token_path);
    }

    if (fp != NULL) {
        /* read the token from saved file */
        size_t read_num = fread(token, 1, sizeof(sgx_launch_token_t), fp);
        if (read_num != 0 && read_num != sizeof(sgx_launch_token_t)) {
            /* if token is invalid, clear the buffer */
            memset(&token, 0x0, sizeof(sgx_launch_token_t));
            printf("Warning: Invalid launch token read from \"%s\".\n", token_path);
        }
    }
    /* Step 2: call sgx_create_enclave to initialize an enclave instance */
    /* Debug Support: set 2nd parameter to 1 */
    ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, &token, &updated, &global_eid, NULL);
    if (ret != SGX_SUCCESS) {
        print_error_message(ret);
        if (fp != NULL) fclose(fp);
        return -1;
    }

    /* Step 3: save the launch token if it is updated */
    if (updated == FALSE || fp == NULL) {
        /* if the token is not updated, or file handler is invalid, do not perform saving */
        if (fp != NULL) fclose(fp);
        return 0;
    }

    /* reopen the file with write capablity */
    fp = freopen(token_path, "wb", fp);
    if (fp == NULL) return 0;
    size_t write_num = fwrite(token, 1, sizeof(sgx_launch_token_t), fp);
    if (write_num != sizeof(sgx_launch_token_t))
        printf("Warning: Failed to save launch token to \"%s\".\n", token_path);
    fclose(fp);
    return 0;
}

#else
# include "../Enclave/Enclave.h"
#endif //End NENCLAVE

/* OCall functions */

long GetFileSize(std::string filename)
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

/*
/// Receives data in to buffer until bufferSize value is met
int RecvBuffer(SOCKET s, char* buffer, int bufferSize, int chunkSize = 4 * 1024) {
    int i = 0;
    while (i < bufferSize) {
        const int l = recv(s, &buffer[i], __min(chunkSize, bufferSize - i), 0);
        if (l < 0) { return l; } // this is an error
        i += l;
    }
    return i;
}
*/

// Sends data in buffer until bufferSize value is met
// int SendBuffer(SOCKET s, const char* buffer, int bufferSize, int chunkSize = 4 * 1024) {
//
//     int i = 0;
//     while (i < bufferSize) {
//         const int l = send(s, &buffer[i], __min(chunkSize, bufferSize - i), 0);
//         if (l < 0) { return l; } // this is an error
//         i += l;
//     }
//     return i;
// }

// Sends a file
// returns size of file if success
// returns -1 if file couldn't be opened for input
// returns -2 if couldn't send file length properly
// returns -3 if file couldn't be sent properly
// https://stackoverflow.com/questions/63494014/sending-files-over-tcp-sockets-c-windows
/*
int64_t SendFile(SOCKET s, const std::string& fileName, int chunkSize = 64 * 1024) {

    const int64_t fileSize = GetFileSize(fileName);
    if (fileSize < 0) { return -1; }

    std::ifstream file(fileName, std::ifstream::binary);
    if (file.fail()) { return -1; }

    if (SendBuffer(s, reinterpret_cast<const char*>(&fileSize),
        sizeof(fileSize)) != sizeof(fileSize)) {
        return -2;
    }

    char* buffer = new char[chunkSize];
    bool errored = false;
    int64_t i = fileSize;
    while (i != 0) {

        const int64_t ssize = __min(i, (int64_t)chunkSize);
        if (!file.read(buffer, ssize)) { errored = true; break; }
        const int l = SendBuffer(s, buffer, (int)ssize);
        if (l < 0) { errored = true; break; }
        i -= l;

    }
    delete[] buffer;

    file.close();

    return errored ? -3 : fileSize;
}
*/

void ocall_print_string(const char* str){
    cout << str;
}

void ocall_print_hash(const char* str, unsigned int n){
  //cout << "Hash of message sent: ";
  //cout.write(str, n);
  //cout << " " << endl;
}
void ocall_print_bytes(const char* str, unsigned int n){
    cout.write(str, n);
}



// this function is not the actual implementation - ignore this
void ocall_setup_connection(){

  int port = PORT;
  char msg[1500];

  //setup a socket and connection tools
    sockaddr_in servAddr;
    bzero((char*)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);

    //open stream oriented socket with internet address
    //also keep track of the socket descriptor
    int serverSd = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSd < 0)
    {
        cerr << "Error establishing the server socket" << endl;
        exit(0);
    }
    //bind the socket to its local address
    int bindStatus = bind(serverSd, (struct sockaddr*) &servAddr,
        sizeof(servAddr));
    if(bindStatus < 0)
    {
        cerr << "Error binding socket to local address" << endl;
        exit(0);
    }
    char* buff = "Hash data for buffer\n";
    ocall_print_string(buff);
    cout << "Waiting for a client to connect...\n" << endl;
    //listen for up to 5 requests at a time


    listen(serverSd, 5);
    //receive a request from client using accept
    //we need a new address to connect with the client
    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    //accept, create a new socket descriptor to
    //handle the new connection with client
    int newSd = accept(serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);
    if(newSd < 0)
    {
        cerr << "Error accepting request from client!" << endl;
        exit(1);
    }
    cout << "Connected with client!" << endl;

    //lets keep track of the session time
    struct timeval start1, end1;
    gettimeofday(&start1, NULL);
    //also keep track of the amount of data sent as well
    int bytesRead, bytesWritten = 0;

    while(1)
    {
        //receive a message from the client (listen)
        cout << "Awaiting client response..." << endl;
        memset(&msg, 0, sizeof(msg));//clear the buffer
        bytesRead += recv(newSd, (char*)&msg, sizeof(msg), 0);
        if(!strcmp(msg, "exit"))
        {
            cout << "Client has quit the session" << endl;
            break;
        }
        cout << "Client: " << msg << endl;
        cout << ">";
        string data;
        getline(cin, data);
        memset(&msg, 0, sizeof(msg)); //clear the buffer
        strcpy(msg, data.c_str());

        ecall_update_hash(global_eid, (char*)&msg);

        if(data == "exit")
        {
            //send to the client that server has closed the connection
            send(newSd, (char*)&msg, strlen(msg), 0);
            break;
        }

        if(data == "sendfile")
        {
          const int64_t fileSize = GetFileSize("example.txt");
          if (fileSize < 0) { cout << "File size is 0" << endl; break; }

          std::ifstream file("example.txt", std::ifstream::binary);
          if (file.fail()) { cout << "Could not read the file. " << endl; break; }

          int64_t chunkSize = 8192; // Bytes

          char* buffer = new char[chunkSize];
          bool errored = false;
          int64_t i = fileSize;

          send(newSd, &fileSize, sizeof(fileSize), 0);

          while (i != 0) {
              const int64_t ssize = min(i, (int64_t)chunkSize);
              if (!file.read(buffer, ssize)) { errored = true; break; }

              //const int l = SendBuffer(s, buffer, (int)ssize);


              int j = 0;
              int l = 0;
              while (j < ssize) {
                  ecall_update_hash(global_eid, (char*)buffer[j]);
                  l = send(newSd, &buffer[j], min(chunkSize, ssize - j), 0);
                  std::cout << "sending : " << j << std::endl;
                  if (l < 0) { j = l; break; } // this is an error
                  j += l;
              }

              if (l < 0) { errored = true; break; }
              i -= l;
          }

          delete[] buffer;
          file.close();
          cout << "File sent." << endl;
        }
        //send the message to client
        bytesWritten += send(newSd, (char*)&msg, strlen(msg), 0);
    }
    //we need to close the socket descriptors after we're all done
    gettimeofday(&end1, NULL);
    close(newSd);
    close(serverSd);
    cout << "********Session********" << endl;
    cout << "Bytes written: " << bytesWritten << " Bytes read: " << bytesRead << endl;
    cout << "Elapsed time: " << (end1.tv_sec - start1.tv_sec)
        << " secs" << endl;
    cout << "Connection closed..." << endl;

}


void mat_multiply_no_sgx(int **mat1,
              int **mat2,
              int **res, int N)
{
    int i, j, k;
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            res[i][j] = 0;
            for (k = 0; k < N; k++){
              int point1 = 0, point2 = 0;
              point1 = mat1[i][k] * mat2[k][j];
              point2 = res[i][j] + point1;
              res[i][j] = point2;
              //res[i][j] += mat1[i][k] * mat2[k][j];
            }
            //cout << res[i][j] << " ";
        }

    }
}


void mat_multiply_with_sgx(int **mat1,
              int **mat2,
              int **res, int N)
{
    int i, j, k;
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            res[i][j] = 0;
            for (k = 0; k < N; k++){
              int point1 = 0, point2 = 0;
              sgx_status_t status1 = mult_enclave(global_eid, &point1, mat1[i][k], mat2[k][j]);
              sgx_status_t status = add_enclave(global_eid, &point2, res[i][j], point1);
              res[i][j] = point2;


            }
            //cout << res[i][j] << " ";
        }

    }
}


//void ocall_send_bytes(){
  // set up socket connection

//}
/* Application entry */
#ifdef NOENCLAVE
int main(int argc, char ** argv)
#else
int SGX_CDECL main(int argc, char *argv[])
#endif
{
    (void)(argc);
    (void)(argv);

#ifndef NOENCLAVE
    /* Initialize the enclave */
    if(initialize_enclave() < 0){
        printf("Enter a character before exit ...\n");
        getchar();
        return -1;
    }
#endif


 // from here matmul
 //int a = 34, b = 23, point1 = 0, point2 = 0;
 //sgx_status_t status = add_enclave(global_eid, &point, a, b);
 //printf("%d\n", point);

 //sgx_status_t status1 = mult_enclave(global_eid, &point2, a, b);
 //printf("%d\n", point2);

    int N = 256;
    srand(0);
    int **res = new int*[N]; // To store result
    int **mat1 = new int*[N]; // To store result
    int **mat2 = new int*[N]; // To store result
    for(int i =0; i < N; ++i)
    {
      res[i]= new int[N];
      mat1[i]= new int[N];
      mat2[i]= new int[N];
      for(int j = 0; j < N; j++)
      {
	       res[i][j] = 0;
	       mat1[i][j] = 1;//(rand()%1000);
	       mat2[i][j] = 2;//(rand()%1000);
      }
    }

    // make NB equal to N for same size matrices
    int NB = 256;
    float a[NB*NB], b[NB*NB], c[NB*NB];

    // initialize matrices a and b
    for (int i = 0; i < NB; i++) {
        for (int j = 0; j < NB; j++) {
            c[i*NB+j] = 0.0; // initialize result matrix c[
            a[i*NB+j] = 1.0;
            b[i*NB+j] = 2.0;
        }
    }

   

    // perform matrix multiplication
    ecall_matmul_u(global_eid, a, b, c, NB);

     // print out the result
    for (int i = 0; i < NB; i++) {
        for (int j = 0; j < NB; j++) {
            cout << c[i*NB+j] << " ";
        }
        cout << endl;
    }

    //mat_multiply_with_sgx(mat1, mat2, res, N);
    mat_multiply_no_sgx(mat1, mat2, res, N);

       // print out the result
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            cout << res[i][j] << " ";
        }
        cout << endl;
    }
    
// END OF MATMUL

/*
// from there the file server code

    ecall_setup_sha256(global_eid);
    //e_call_setup_connection(global_eid);

    int port = PORT;
    char msg[1500];


  // add_in_enclave(global_eid, a,b,&sum, 5);

   //setup a socket and connection tools
     sockaddr_in servAddr;
     bzero((char*)&servAddr, sizeof(servAddr));
     servAddr.sin_family = AF_INET;
     servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
     servAddr.sin_port = htons(port);

     //open stream oriented socket with internet address
     //also keep track of the socket descriptor
     int serverSd = socket(AF_INET, SOCK_STREAM, 0);
     if(serverSd < 0)
     {
         cerr << "Error establishing the server socket" << endl;
         exit(0);
     }
     //bind the socket to its local address
     int bindStatus = bind(serverSd, (struct sockaddr*) &servAddr,
         sizeof(servAddr));
     if(bindStatus < 0)
     {
         cerr << "Error binding socket to local address" << endl;
         exit(0);
     }

     //cout << "Waiting for a client to connect...\n" << endl;
     //listen for up to 5 requests at a time


     if (listen(serverSd, 3) < 0) {
         perror("[ERROR] : Listen");
         exit(EXIT_FAILURE);
     }
     //cout<<"[LOG] : Socket in Listen State (Max Connection Queue: 3)\n";
     //receive a request from client using accept
     //we need a new address to connect with the client
     sockaddr_in newSockAddr;
     socklen_t newSockAddrSize = sizeof(newSockAddr);
     //accept, create a new socket descriptor to
     //handle the new connection with client

     int newSd = accept(serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);
     if(newSd < 0)
     {
         cerr << "Error accepting request from client!" << endl;
         exit(1);
     }

     //cout<<"[LOG] : Connected to Client.\n";

     string fileLoc = "ubuntu-22.04.1-desktop-amd64.iso";
     fstream file1;

     file1.open(fileLoc, ios::in | ios::binary);
     if(file1.is_open()){
         //cout<<"[LOG] : File is ready to Transmit.\n";
     }
     else{
         cout<<"[ERROR] : File loading failed, Exititng.\n";
         exit(EXIT_FAILURE);
     }
     //auto start = chrono::high_resolution_clock::now(); //time

     const int64_t fileSize = GetFileSize(fileLoc);
     if (fileSize < 0) { cout << "File size is 0" << endl; }

     std::ifstream file(fileLoc, std::ifstream::binary);
     if (file.fail()) { cout << "Could not read the file. " << endl; }

     int64_t chunkSize = 8192;

     char* buffer = new char[chunkSize];
     bool errored = false;
     int64_t i = fileSize;

     send(newSd, &fileSize, sizeof(fileSize), 0);

     while (i != 0) {
         const int64_t ssize = min(i, (int64_t)chunkSize);
         if (!file.read(buffer, ssize)) { errored = true; break; }

         //const int l = SendBuffer(s, buffer, (int)ssize);

         int j = 0;
         int l = 0;
         while (j < ssize) {
             ecall_update_hash(global_eid, (char*)&buffer[j]);
             l = send(newSd, &buffer[j], min(chunkSize, ssize - j), 0);
             if (l < 0) { j = l; break; } // this is an error
             j += l;
         }

         if (l < 0) { errored = true; break; }
         i -= l;
     }

     delete[] buffer;
     file.close();
     //cout << "File sent from server!" << endl;

     //lets keep track of the session time
    // struct timeval start1, end1;
     // gettimeofday(&start1, NULL);
     // //also keep track of the amount of data sent as well
     // int bytesRead, bytesWritten = 0;
     // //
     // while(1)
     // {
     //     //receive a message from the client (listen)
     //     cout << "Awaiting client response..." << endl;
     //     memset(&msg, 0, sizeof(msg));//clear the buffer
     //     bytesRead += recv(newSd, (char*)&msg, sizeof(msg), 0);
     //     if(!strcmp(msg, "exit"))
     //     {
     //         cout << "Client has quit the session" << endl;
     //         break;
     //     }
     //     cout << "Client: " << msg << endl;
     //     cout << ">";
     //     string data;
     //     getline(cin, data);
     //     memset(&msg, 0, sizeof(msg)); //clear the buffer
     //     strcpy(msg, data.c_str());
     //
     //     ecall_update_hash(global_eid, (char*)&msg);
     //
     //     if(data == "exit")
     //     {
     //         //send to the client that server has closed the connection
     //         send(newSd, (char*)&msg, strlen(msg), 0);
     //         break;
     //     }
     //
     //     if(data == "sendfile")
     //     {
     //       const int64_t fileSize = GetFileSize("ubuntu-22.04.1-desktop-amd64.iso");
     //       if (fileSize < 0) { cout << "File size is 0" << endl; break; }
     //
     //       std::ifstream file("ubuntu-22.04.1-desktop-amd64.iso", std::ifstream::binary);
     //       if (file.fail()) { cout << "Could not read the file. " << endl; break; }
     //
     //       int64_t chunkSize = 1024;
     //
     //       char* buffer = new char[chunkSize];
     //       bool errored = false;
     //       int64_t i = fileSize;
     //
     //       send(newSd, &fileSize, sizeof(fileSize), 0);
     //
     //       while (i != 0) {
     //           const int64_t ssize = min(i, (int64_t)chunkSize);
     //           if (!file.read(buffer, ssize)) { errored = true; break; }
     //
     //           //const int l = SendBuffer(s, buffer, (int)ssize);
     //
     //
     //           int j = 0;
     //           int l = 0;
     //           while (j < ssize) {
     //               ecall_update_hash(global_eid, (char*)&buffer[j]);
     //               l = send(newSd, &buffer[j], min(chunkSize, ssize - j), 0);
     //               if (l < 0) { j = l; break; } // this is an error
     //               j += l;
     //           }
     //
     //           if (l < 0) { errored = true; break; }
     //           i -= l;
     //       }
     //
     //       delete[] buffer;
     //       file.close();
     //       cout << "File sent." << endl;
     //     }
     //     //send the message to client
     //     bytesWritten += send(newSd, (char*)&msg, strlen(msg), 0);
     // }
     //we need to close the socket descriptors after we're all done
     // gettimeofday(&end1, NULL);
     // close(newSd);
     // close(serverSd);
     // cout << "********Session********" << endl;
     // cout << "Bytes written: " << bytesWritten << " Bytes read: " << bytesRead << endl;
     // cout << "Elapsed time: " << (end1.tv_sec - start1.tv_sec)
     //     << " secs" << endl;
     // cout << "Connection closed..." << endl;

/*
   char* str = "Kryptonite is the weakness";

#ifndef NOENCLAVE
    add_in_enclave(global_eid, num1, num2, &sum, len);
    sub_in_enclave(global_eid, num1, num2, &sub, len);
    e_call_hello(global_eid, str);
#else
    add_in_enclave(num1, num2, &sum, len);
    sub_in_enclave(num1, num2, &sub, len);
    e_call_hello(str);
#endif

    printf("Returned from add_in_enclave ecall\nSum computed is %i + %i = %i\n", num1, num2, sum);
    printf("Returned from sub_in_enclave ecall\nSub computed is %i - %i = %i\n", num1, num2, sub);
    printf("Returned from e_call_hello ecall\n");
*/

#ifndef NOENCLAVE
    /* Destroy the enclave */
    sgx_destroy_enclave(global_eid);
#endif

/* //from here time
    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
    //cout << duration.count() << endl;
    ofstream outfile;
    outfile.open("sc1_time.txt", ios_base::app);
    outfile << to_string(duration.count()) + "\n";*/
    return 0;
}
