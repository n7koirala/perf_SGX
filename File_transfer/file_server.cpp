#include<iostream>
#include<fstream>
#include<stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <chrono> //for time

using namespace std;

#define SHA256_DIGEST_LENGTH 32

class Server_socket{
    fstream file;

    int PORT;

    int general_socket_descriptor;
    int new_socket_descriptor;

    struct sockaddr_in address;
    int address_length;

    string fileLoc = "./Data/Server/ubuntu-22.04.1-desktop-amd64.iso";

    public:
        Server_socket(){
            create_socket();
            PORT = 8060;

            address.sin_family = AF_INET;
            address.sin_addr.s_addr = INADDR_ANY;
            address.sin_port = htons( PORT );
            address_length = sizeof(address);

            bind_socket();
            set_listen_set();
            accept_connection();


            file.open(fileLoc, ios::in | ios::binary);
            if(file.is_open()){
                //cout<<"[LOG] : File is ready to Transmit.\n";
            }
            else{
                cout<<"[ERROR] : File loading failed, Exititng.\n";
                exit(EXIT_FAILURE);
            }
        }



        void create_socket(){
            if ((general_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
                perror("[ERROR] : Socket failed");
                exit(EXIT_FAILURE);
            }
            //cout<<"[LOG] : Socket Created Successfully.\n";
        }

        void bind_socket(){
            if (bind(general_socket_descriptor, (struct sockaddr *)&address, sizeof(address))<0) {
                perror("[ERROR] : Bind failed");
                exit(EXIT_FAILURE);
            }
            //cout<<"[LOG] : Bind Successful.\n";
        }

        void set_listen_set(){
            if (listen(general_socket_descriptor, 3) < 0) {
                perror("[ERROR] : Listen");
                exit(EXIT_FAILURE);
            }
            //cout<<"[LOG] : Socket in Listen State (Max Connection Queue: 3)\n";
        }

        void accept_connection(){
            if ((new_socket_descriptor = accept(general_socket_descriptor, (struct sockaddr *)&address, (socklen_t*)&address_length))<0) {
                perror("[ERROR] : Accept");
                exit(EXIT_FAILURE);
            }
            //cout<<"[LOG] : Connected to Client.\n";
        }

        void transmit_file(){
            std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            cout<<"[LOG] : Transmission Data Size "<<contents.length()<<" Bytes.\n";

            cout<<"[LOG] : Sending...\n";

            int bytes_sent = send(new_socket_descriptor , contents.c_str() , contents.length() , 0 );
            cout<<"[LOG] : Transmitted Data Size "<<bytes_sent<<" Bytes.\n";

            cout<<"[LOG] : File Transfer Complete.\n";
        }

        long GetFileSize(std::string filename)
        {
            struct stat stat_buf;
            int rc = stat(filename.c_str(), &stat_buf);
            return rc == 0 ? stat_buf.st_size : -1;
        }

        void transmit_image(){
          const int64_t fileSize = GetFileSize(fileLoc);
          if (fileSize < 0) { cout << "File size is 0" << endl; }

          std::ifstream file(fileLoc, std::ifstream::binary);
          if (file.fail()) { cout << "Could not read the file. " << endl; }

          int64_t chunkSize = 8192;

          char* buffer = new char[chunkSize];
          bool errored = false;
          int64_t i = fileSize;

          send(new_socket_descriptor, &fileSize, sizeof(fileSize), 0);

          while (i != 0) {
              const int64_t ssize = min(i, (int64_t)chunkSize);
              if (!file.read(buffer, ssize)) { errored = true; break; }

              //const int l = SendBuffer(s, buffer, (int)ssize);

              int j = 0;
              int l = 0;
              while (j < ssize) {
                  sha256((char*)&buffer[j]);
                  l = send(new_socket_descriptor, &buffer[j], min(chunkSize, ssize - j), 0);
                  if (l < 0) { j = l; break; } // this is an error
                  j += l;
              }

              if (l < 0) { errored = true; break; }
              i -= l;
          }

          delete[] buffer;
          file.close();
          //cout << "File sent from server!" << endl;
        }

        void sha256(char* buffer)
        {
          // Initialize SHA256 context
          SHA256_CTX ctx;
          SHA256_Init(&ctx);

          // Update context with buffer data
          SHA256_Update(&ctx, buffer, strlen(buffer));

          // Finalize hash computation
          unsigned char hash[SHA256_DIGEST_LENGTH];
          SHA256_Final(hash, &ctx);

          // Print hash value
          //cout << "Hash value: ";
          //for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            //printf("%02x", hash[i]);
          //}
          //cout << endl;
        }
};

int main(){
    Server_socket S;
    auto start = chrono::high_resolution_clock::now();
    S.transmit_image();
    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
    //cout << duration.count() << endl;
    ofstream outfile;
    outfile.open("sc1_time.txt", ios_base::app);
    outfile << to_string(duration.count()) + "\n";
    return 0;
}
