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
using namespace std;

#define PORT 8080
#define SNIPPETLENGTH 11
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

void writeFiles(int client_fd, string* fileNames) {
  char snippet[SNIPPETLENGTH+1] = "";
  for (int i = 0; i < 5; i++) {
    cout<<"Writing File #"<<(i+1)<<endl;
    ifstream currentFile(*(fileNames + i));
    while(true) {
      currentFile.read(snippet, sizeof(snippet) - 1);
      send(client_fd, snippet, strlen(snippet), 0);
      if(currentFile.eof()) {
        break;
      }
    }
    currentFile.close();
    send(client_fd, "End of File", strlen("End of File"), 0);
    cout<<"Finished writing File #"<<(i+1)<<endl;
  }
}

void readFiles(int client_fd) {
  ssize_t valread;
  char buffer[SNIPPETLENGTH+1] = { 0 };
  string readFileNames[5] = {"./CopiedFilesFromServer/serverFile1.txt", "./CopiedFilesFromServer/serverFile2.txt",
                             "./CopiedFilesFromServer/serverFile3.txt",
                             "./CopiedFilesFromServer/serverFile4.txt", "./CopiedFilesFromServer/serverFile5.txt"};
  mkdir("./CopiedFilesFromServer/", 0777);
  for (int i = 0; i < 5; i++) {
    remove(readFileNames[i].c_str());
    cout<<"Reading File #"<<(i+1)<<endl;
    ofstream of;
    of.open(readFileNames[i], ios::app);
    do {
      valread = read(client_fd, buffer,
                     SNIPPETLENGTH); // subtract 1 for the null
                                     // terminator at the end
      if (strcmp(buffer, "End of File") != 0) {
        of << buffer;
      }
    } while (strcmp(buffer, "End of File") != 0);
    of.close();
    cout<<"File #"<<(i+1)<<" Read!"<<endl;
  }
}

void formatFiles(string* fileNames) {
  char buffer[SNIPPETLENGTH+1] = { 0 };
  string readFileNames[5] = {"./CopiedFilesFromServer/clientFile1.txt", "./CopiedFilesFromServer/clientFile2.txt",
                             "./CopiedFilesFromServer/clientFile3.txt",
                             "./CopiedFilesFromServer/clientFile4.txt", "./CopiedFilesFromServer/clientFile5.txt"};
  mkdir("./CopiedFilesFromServer/", 0777);
  for (int i = 0; i < 5; i++) {
    remove(readFileNames[i].c_str());
    cout<<"Formatting File #"<<(i+1)<<endl;
    ofstream of;
    ifstream currentFile(*(fileNames + i));
    of.open(readFileNames[i], ios::app);
    do {
      currentFile.read(buffer, sizeof(buffer) - 1);
      of << buffer;
    } while (!currentFile.eof());
    of.close();
    currentFile.close();
    cout<<"File #"<<(i+1)<<" Formatted!"<<endl;
  }
}

bool compareFiles(string fileName1, string fileName2) {
  ifstream file1(fileName1);
  ifstream file2(fileName2);
  char buffer1[4096];
  char buffer2[4096];
  do {
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
    writeFiles(client_fd, fileNames);
    readFiles(client_fd);
    close(client_fd);
    formatFiles(fileNames);
    cout<<"Are files different?: "<<boolalpha<<areAllFilesDifferent()<<endl;
    return 0;
}
