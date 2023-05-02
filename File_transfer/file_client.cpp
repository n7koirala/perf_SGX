#include<iostream>
#include<fstream>
#include<stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

class Client_socket{
    fstream file;

    int PORT;

    int general_socket_descriptor;

    struct sockaddr_in address;
    int address_length;

    public:
        Client_socket(){
            create_socket();
            PORT = 8060;

            address.sin_family = AF_INET;
            address.sin_port = htons( PORT );
            address_length = sizeof(address);
            if(inet_pton(AF_INET, "127.0.0.1", &address.sin_addr)<=0) {
                cout<<"[ERROR] : Invalid address\n";
            }

            create_connection();

            file.open("./Data/Client/received_image.iso", ios::out | ios::trunc | ios::binary);
            if(file.is_open()){
                cout<<"[LOG] : File Created.\n";
            }
            else{
                cout<<"[ERROR] : File creation failed, Exititng.\n";
                exit(EXIT_FAILURE);
            }
        }

        void create_socket(){
            if ((general_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                perror("[ERROR] : Socket failed.\n");
                exit(EXIT_FAILURE);
            }
            //cout<<"[LOG] : Socket Created Successfully.\n";
        }

        void create_connection(){
            if (connect(general_socket_descriptor, (struct sockaddr *)&address, sizeof(address)) < 0) {
                perror("[ERROR] : connection attempt failed.\n");
                exit(EXIT_FAILURE);
            }
            //cout<<"[LOG] : Connection Successfull.\n";
        }

        void receive_file(){
            char buffer[1024] = {};
            int valread = read(general_socket_descriptor , buffer, 1024);
            cout<<"[LOG] : Data received "<<valread<<" bytes\n";
            cout<<"[LOG] : Saving data to file.\n";

            file<<buffer;
            cout<<"[LOG] : File Saved.\n";
        }

        void receive_image(){
          std::ofstream file("Data/Client/received_image.iso", std::ofstream::binary);
          if (file.fail()) { cout << "File could not be opened. " << endl; }

          int64_t fileSize; 

          recv(general_socket_descriptor, &fileSize, sizeof(fileSize), 0);

          int64_t chunkSize = 8192;
          char* buffer = new char[chunkSize];
          bool errored = false;
          int64_t i = fileSize;
          while (i != 0) {
              const int64_t ssize = min(i, (int64_t)chunkSize);
              //const int r = RecvBuffer(s, buffer, (int)__min(i, (int64_t)chunkSize));

              int j = 0;
              int l = 0;
              while (j < ssize) {
              l = recv(general_socket_descriptor, &buffer[j], min(chunkSize, ssize - j), 0);
              if (l < 0) { j = l; break; } // this is an error
              j += l;
             }

              if ((l < 0) || !file.write(buffer, l)) { errored = true; break; }
              i -= l;
          }
          delete[] buffer;
          file.close();
          cout << "File received on client!" << endl;

        }
};

int main(){
    Client_socket C;
    C.receive_image();
    return 0;
}
