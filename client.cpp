// Client side C/C++ program to demonstrate Socket
// programming
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
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
#include <limits>
#include <cmath>
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

int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

string* getFileNames() {
  static string fileNames[5];
  string currentFilePath;
  char fullPath1[512];
  char fullPath2[512];
  int fileNumber = 1;
  while (fileNumber < 6) {
    bool validFile = true;
    cout<<"Enter the path to file #"<<to_string(fileNumber)<<":";
    cin>>currentFilePath;
    if (currentFilePath.empty()) {
      validFile = false;
    } else {
      ifstream currentFileTest(currentFilePath);
      if (!currentFileTest.good()) {
        validFile = false;
        currentFileTest.close();
      } else {
        currentFileTest.close();
        const char* currentFilePathPointer = currentFilePath.c_str();
        char* checkPointer = realpath(currentFilePathPointer, fullPath1);
        if (is_regular_file(fullPath1) != 1) {
          validFile = false;
        } else {
          for (int i = 0; i < fileNumber-1; i++) {
            const char* fileNamesPointer = fileNames[i].c_str();
            char* checkPointer2 = realpath(fileNamesPointer, fullPath2);
            if (checkPointer == NULL || checkPointer2 == NULL || currentFilePath == fileNames[i] || strcmp(fullPath1, fullPath2) == 0) {
              validFile = false;
              break;
            }
          }
        }
      }
    }
    if (validFile) {
      fileNames[fileNumber - 1] = currentFilePath;
      fileNumber++;
    } else {
      cout<<"Error, file path was not valid!"<<endl;
    }
  }
  return fileNames;
}

string find_shared_string(char buffer[], int numOfComma) {
  string bufferString(buffer);
  int currentComma = 0;
  int currentIndex = 0;
  for (int i = 0; i < strlen(bufferString.c_str()); i++) {
    if (bufferString[i] == ',') {
      if (currentComma == numOfComma) {
        return bufferString.substr(currentIndex, i - currentIndex);
      }
      currentComma++;
      currentIndex = i + 1;
    }
  }
  return bufferString.substr(currentIndex, strlen(bufferString.c_str()) - currentIndex);
}

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
      send(new_socket, bufferString.c_str(), strlen(bufferString.c_str()), 0);
    }
  } while (bufferString.compare("End of File") != 0);
  of.close();
}

size_t find_file_size(string fileName) {
  ifstream file;
  file.open(fileName, ios::in|ios::binary);
  file.ignore( std::numeric_limits<std::streamsize>::max() );
  std::streamsize length = file.gcount();
  file.clear();   //  Since ignore will have set eof.
  file.seekg( 0, std::ios_base::beg );
  file.close();
  return length;
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
    send(new_socket, send_message.data(), send_message.length(), 0);
    
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

void write_secret_keys(int new_socket, unsigned long long shared_q, unsigned long long shared_g, string gamalString) {
  string keysString = to_string(shared_q) + "," + to_string(shared_g) + "," + gamalString;
  send(new_socket, keysString.c_str(), strlen(keysString.c_str()), 0);
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

void writeFiles(int client_fd, string* fileNames, unsigned long long secret_encrypt_s) {
  char snippet[SNIPPETLENGTH+1] = "";
  string readFileNames[5] = {"./CopiedFilesFromServer/clientFile1.txt", "./CopiedFilesFromServer/clientFile2.txt",
                             "./CopiedFilesFromServer/clientFile3.txt",
                             "./CopiedFilesFromServer/clientFile4.txt", "./CopiedFilesFromServer/clientFile5.txt"};
  mkdir("./CopiedFilesFromServer/", 0777);
  for (int i = 0; i < 5; i++) {
    remove(readFileNames[i].c_str());
    cout<<"Writing File #"<<(i+1)<<endl;
    writeElGamal(client_fd, *(fileNames + i), readFileNames[i], secret_encrypt_s);
    cout<<"Finished writing File #"<<(i+1)<<endl;
  }
}

void readFiles(int client_fd, unsigned long long secret_encrypt_s, unsigned long long secret_gamal_key) {
  ssize_t valread;
  char buffer[SNIPPETLENGTH+1] = { 0 };
  string readFileNames[5] = {"./CopiedFilesFromServer/serverFile1.txt", "./CopiedFilesFromServer/serverFile2.txt",
                             "./CopiedFilesFromServer/serverFile3.txt",
                             "./CopiedFilesFromServer/serverFile4.txt", "./CopiedFilesFromServer/serverFile5.txt"};
  mkdir("./CopiedFilesFromServer/", 0777);
  for (int i = 0; i < 5; i++) {
    remove(readFileNames[i].c_str());
    cout<<"Reading File #"<<(i+1)<<endl;
    readElGamal(client_fd, readFileNames[i], secret_encrypt_s, secret_gamal_key);
    cout<<"File #"<<(i+1)<<" Read!"<<endl;
  }
}

bool compareFiles(string fileName1, string fileName2) {
  ifstream file1(fileName1);
  ifstream file2(fileName2);
  char buffer1[4096] = { 0 };
  char buffer2[4096] = { 0 };
  do {
    memset(buffer1, 0, sizeof buffer1);
    memset(buffer2, 0, sizeof buffer2);
    file1.read(buffer1, sizeof(buffer1) - 1);
    file2.read(buffer2, sizeof(buffer2) - 1);
    if (strcmp(buffer1, buffer2) != 0) {
      file1.close();
      file2.close();
      return false;
    }
  } while (!file1.eof() && !file2.eof());
  if (file1.eof() && file2.eof()) {
    file1.close();
    file2.close();
    return true;
  }
  file1.close();
  file2.close();
  return false;
}

bool areAllFilesDifferent() {
  string serverFiles[5] = {"./CopiedFilesFromServer/serverFile1.txt", "./CopiedFilesFromServer/serverFile2.txt",
                           "./CopiedFilesFromServer/serverFile3.txt",
                           "./CopiedFilesFromServer/serverFile4.txt", "./CopiedFilesFromServer/serverFile5.txt"};
  string clientFiles[5] = {"./CopiedFilesFromServer/clientFile1.txt", "./CopiedFilesFromServer/clientFile2.txt",
                             "./CopiedFilesFromServer/clientFile3.txt",
                             "./CopiedFilesFromServer/clientFile4.txt", "./CopiedFilesFromServer/clientFile5.txt"};
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      if (compareFiles(serverFiles[i], clientFiles[j])) {
        return false;
      }
    }
  }
  return true;
}

int main(int argc, char const* argv[])
{
    srand((unsigned)time(NULL));
    //cout<<pow(3,3)<<endl;
    string* fileNames = getFileNames();
    for (int i = 0; i < 5; i++) {
      cout<<"File Name #"<<(i + 1)<<": "<<*(fileNames + i)<<endl;
    }
    cout<<"Found all files!"<<endl;
    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    cout<<"Waiting for Server to connect!"<<endl;
    do {
      if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
      }
      serv_addr.sin_family = AF_INET;
      serv_addr.sin_port = htons(PORT);

      // Convert IPv4 and IPv6 addresses from text to binary
      // form
      if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)
          <= 0) {
          printf(
              "\nInvalid address/ Address not supported \n");
          return -1;
      }
      status = connect(client_fd, (struct sockaddr*)&serv_addr,
                   sizeof(serv_addr));
      if (status < 0) {
        close(client_fd);
      }
    } while (status < 0);
    cout<<"Connected to server!"<<endl;
    
    read_secret_keys(client_fd);
    unsigned long long secret_encrypt_key = gen_key(global_shared_encrypt_Q);
    
    //cout<<"Shared Q: "<<global_shared_encrypt_Q<<endl;
    //cout<<"Shared G: "<<global_shared_encrypt_G<<endl;
    //cout<<"Secret key: "<<secret_encrypt_key<<endl;
    writeFiles(client_fd, fileNames, secret_encrypt_key);
    
    global_shared_gamal_Q = gen_rand();
    unsigned long long secret_gamal_Key = gen_key(global_shared_gamal_Q);
    global_shared_gamal_G = gen_g();
    global_shared_gamal_H = power(global_shared_gamal_G, secret_gamal_Key, global_shared_gamal_Q);
    string initial_el_gamal_string = send_init_gamal(global_shared_gamal_Q, global_shared_gamal_G, global_shared_gamal_H);
    
    write_secret_keys(client_fd, global_shared_encrypt_Q, global_shared_encrypt_G, initial_el_gamal_string);
    //cout<<"Shared Q: "<<global_shared_encrypt_Q<<endl;
    //cout<<"Shared G: "<<global_shared_encrypt_G<<endl;
    //cout<<"Secret key: "<<secret_encrypt_key<<endl;
    readFiles(client_fd, secret_encrypt_key, secret_gamal_Key);
    
    close(client_fd);
    cout<<"Are files different?: "<<boolalpha<<areAllFilesDifferent()<<endl;
    
    return 0;
}
