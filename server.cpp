#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <filesystem>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdio>
#include "gamal.h"
using namespace std;

#define PORT 8080
#define SNIPPETLENGTH 8
#define MAXLENGTH 5000

unsigned long long global_h2;
unsigned long long global_shared_encrypt_Q;
unsigned long long global_shared_encrypt_G;
unsigned long long global_shared_gamal_Q;
unsigned long long global_shared_gamal_G;
unsigned long long global_shared_gamal_H;

string splitAndDecryptElGamal(char* buffer, unsigned long long secretKey, unsigned long long sharedQ) {
  string bufferString(buffer);
  string h2;
  string keyAndMessage;
  if (bufferString.compare("End of File") == 0) {
    return "End of File";
  }
  for (int i = 0; i < strlen(bufferString.c_str()); i++) {
    if (bufferString[i] == ',') {
      h2 = bufferString.substr(0, i);
      global_h2 = stoull(h2);
      keyAndMessage = bufferString.substr(i + 1, strlen(bufferString.c_str()) - (i + 1));
      break;
    }
  }
  return decrypt(keyAndMessage, get_key(stoull(h2), secretKey, sharedQ));
}

void readElGamal(int new_socket, string fileName, unsigned long long secretS, unsigned long long secretElGamalKey) {
  ssize_t valread;
  ofstream of;
  of.open(fileName, ios::app);
  string bufferString;
  char buffer[MAXLENGTH+1] = { 0 };
  do {
    memset(buffer, 0, sizeof buffer);
    valread = read(new_socket, buffer, MAXLENGTH);
    bufferString = buffer;
    unsigned long long recieverKey = get_key(stoull(find_shared_string(buffer, 0)), secretElGamalKey, global_shared_gamal_Q);
    bufferString = decrypt(find_shared_string(buffer, 1), recieverKey);
    if (bufferString.compare("End of File") != 0) {
      
      bufferString = secret_final_encrypt(bufferString, secretS, global_shared_encrypt_Q);
      of << (bufferString + "|");
      bufferString = encrypt(bufferString, recieverKey);
      send(new_socket, bufferString.data(), bufferString.length(), 0);
    }
  } while (bufferString.compare("End of File") != 0);
  of.close();
}

void writeElGamal(int new_socket, string fileName, string outFileName, unsigned long long secret_encrypt_s) {
  size_t fileSize = find_file_size(fileName);
  size_t currentSize = 0;
  ssize_t valread;
  int percent = 0;
  ofstream of;
  of.open(outFileName, ios::app);
  ifstream currentFile(fileName);
  char snippet[SNIPPETLENGTH+1] = { 0 };
  char buffer[MAXLENGTH+1] = { 0 };
  unsigned long long secret_el_gamal_key;
  unsigned long long shared_h2;
  unsigned long long sender_key;
  string send_message;
  while(true) {
    memset(buffer, 0, sizeof buffer);
    memset(snippet, 0, sizeof snippet);

    currentFile.read(snippet, SNIPPETLENGTH - 2);
    if (snippet[0] == 0) {
      break;
    }
    currentSize += strlen(snippet);
    if (percent < floor((double)((double)(currentSize * 1.0) / (double)(fileSize * 1.0)) * 100)) {
      percent = floor((double)((double)(currentSize * 1.0) / (double)(fileSize * 1.0)) * 100);
      cout<<"Percent complete: "<<percent<<"%"<<endl;
    }
    string snippetString(snippet);
    send_message = secret_encrypt(snippetString, secret_encrypt_s, global_shared_encrypt_Q);
    secret_el_gamal_key = gen_key(global_shared_gamal_Q);
    shared_h2 = power(global_shared_gamal_G, secret_el_gamal_key, global_shared_gamal_Q);
    sender_key = power(global_shared_gamal_H, secret_el_gamal_key, global_shared_gamal_Q);
    
    send_message = encrypt(send_message, sender_key);
    send_message = send_first_response(shared_h2, send_message);
    send(new_socket, send_message.c_str(), strlen(send_message.c_str()), 0);

    valread = read(new_socket, buffer, MAXLENGTH);
    string bufferString = buffer;
    bufferString = decrypt(bufferString, sender_key);
    of << (bufferString + "|");
    if(currentFile.eof()) {
      break;
    }
  }
  of.close();
  currentFile.close();
  send_message = "End of File";
  secret_el_gamal_key = gen_key(global_shared_gamal_Q);
  shared_h2 = power(global_shared_gamal_G, secret_el_gamal_key, global_shared_gamal_Q);
  sender_key = power(global_shared_gamal_H, secret_el_gamal_key, global_shared_gamal_Q);

  send_message = encrypt(send_message, sender_key);
  send_message = send_first_response(shared_h2, send_message);
  send(new_socket, send_message.c_str(), strlen(send_message.c_str()), 0);
}

void read_secret_keys(int new_socket) {
  char buffer[MAXLENGTH+1] = { 0 };
  ssize_t valread = read(new_socket, buffer, MAXLENGTH);
  global_shared_encrypt_Q = stoull(find_shared_string(buffer, 0));
  global_shared_encrypt_G = stoull(find_shared_string(buffer, 1));
  global_shared_gamal_Q = stoull(find_shared_string(buffer, 2));
  global_shared_gamal_G = stoull(find_shared_string(buffer, 3));
  global_shared_gamal_H = stoull(find_shared_string(buffer, 4));
}

void readFiles(int client_fd, unsigned long long secret_encrypt_s, unsigned long long secret_gamal_key, string readFileNames[5]) {
  ssize_t valread;
  char buffer[SNIPPETLENGTH+1] = { 0 };
  mkdir("./CopiedFilesFromServer/", 0777);
  for (int i = 0; i < 5; i++) {
    remove(readFileNames[i].c_str());
    cout<<"Reading File #"<<(i+1)<<endl;
    readElGamal(client_fd, readFileNames[i], secret_encrypt_s, secret_gamal_key);
    cout<<"File #"<<(i+1)<<" Read!"<<endl;
  }
}

void writeFiles(int client_fd, string* fileNames, unsigned long long secret_encrypt_s, string writeFileNames[5]) {
  char snippet[SNIPPETLENGTH+1] = "";
  mkdir("./CopiedFilesFromClient/", 0777);
  for (int i = 0; i < 5; i++) {
    remove(writeFileNames[i].c_str());
    cout<<"Writing File #"<<(i+1)<<endl;
    writeElGamal(client_fd, *(fileNames + i), writeFileNames[i], secret_encrypt_s);
    cout<<"Finished writing File #"<<(i+1)<<endl;
  }
}

int main(int argc, char const* argv[])
{
    string serverFiles[5] = {"./CopiedFilesFromServer/serverFile1.txt", "./CopiedFilesFromServer/serverFile2.txt",
                           "./CopiedFilesFromServer/serverFile3.txt",
                           "./CopiedFilesFromServer/serverFile4.txt", "./CopiedFilesFromServer/serverFile5.txt"};
    string clientFiles[5] = {"./CopiedFilesFromServer/clientFile1.txt", "./CopiedFilesFromServer/clientFile2.txt",
                             "./CopiedFilesFromServer/clientFile3.txt",
                             "./CopiedFilesFromServer/clientFile4.txt", "./CopiedFilesFromServer/clientFile5.txt"};
    srand((unsigned)time(NULL));
    string* fileNames = getFileNames();
    for (int i = 0; i < 5; i++) {
      cout<<"File Name #"<<(i + 1)<<": "<<*(fileNames + i)<<endl;
    }
    cout<<"Found all files!"<<endl;
    cout<<"Waiting for Client to connect!"<<endl;
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
 
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
 
    if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 1) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket
         = accept(server_fd, (struct sockaddr*)&address,
                  &addrlen))
        < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    cout<<"Connected to client!"<<endl;
    global_shared_encrypt_Q = gen_rand();
    global_shared_encrypt_G = gen_g();
    unsigned long long secret_encrypt_key = gen_key(global_shared_encrypt_Q);
    
    global_shared_gamal_Q = gen_rand();
    unsigned long long secret_gamal_Key = gen_key(global_shared_gamal_Q);
    global_shared_gamal_G = gen_g();
    global_shared_gamal_H = power(global_shared_gamal_G, secret_gamal_Key, global_shared_gamal_Q);
    string initial_el_gamal_string = send_init_gamal(global_shared_gamal_Q, global_shared_gamal_G, global_shared_gamal_H);
    
    write_secret_keys(new_socket, global_shared_encrypt_Q, global_shared_encrypt_G, initial_el_gamal_string);
    readFiles(new_socket, secret_encrypt_key, secret_gamal_Key, clientFiles);
    
    read_secret_keys(new_socket);
    writeFiles(new_socket, fileNames, secret_encrypt_key, serverFiles);
    close(new_socket);
    close(server_fd);
    cout<<"Are files different?: "<<boolalpha<<areAllFilesDifferent(serverFiles, clientFiles)<<endl;
    return 0;
}
