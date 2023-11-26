
// Server side C/C++ program to demonstrate Socket
// programming
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

void writeFiles(int new_socket, string* fileNames) {
  char snippet[SNIPPETLENGTH+1] = "";
  for (int i = 0; i < 5; i++) {
    cout<<"Writing File #"<<(i+1)<<endl;
    ifstream currentFile(*(fileNames + i));
    while(true) {
      currentFile.read(snippet, sizeof(snippet) - 1);
      send(new_socket, snippet, strlen(snippet), 0);
      if(currentFile.eof()) {
        break;
      }
    }
    currentFile.close();
    send(new_socket, "End of File", strlen("End of File"), 0);
    cout<<"Finished writing File #"<<(i+1)<<endl;
  }
}

void readFiles(int new_socket) {
  ssize_t valread;
  char buffer[SNIPPETLENGTH+1] = { 0 };
  string readFileNames[5] = {"./CopiedFilesFromClient/clientFile1.txt", "./CopiedFilesFromClient/clientFile2.txt",
			     "./CopiedFilesFromClient/clientFile3.txt",
                             "./CopiedFilesFromClient/clientFile4.txt", "./CopiedFilesFromClient/clientFile5.txt"};
  mkdir("./CopiedFilesFromClient/", 0777);
  for (int i = 0; i < 5; i++) {
    remove(readFileNames[i].c_str());
    cout<<"Reading File #"<<(i+1)<<endl;
    ofstream of;
    of.open(readFileNames[i], ios::app);
    do {
      valread = read(new_socket, buffer,
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
  string readFileNames[5] = {"./CopiedFilesFromClient/serverFile1.txt", "./CopiedFilesFromClient/serverFile2.txt",
                             "./CopiedFilesFromClient/serverFile3.txt",
                             "./CopiedFilesFromClient/serverFile4.txt", "./CopiedFilesFromClient/serverFile5.txt"};
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

int main(int argc, char const* argv[])
{
    string* fileNames = getFileNames();
    for (int i = 0; i < 5; i++) {
      cout<<"File Name #"<<(i + 1)<<*(fileNames + i)<<endl;
    }
    cout<<"Found all files!"<<endl;
    // Create a text string, which is used to output the text file
    //string myText;

    // Read from the text file
    //ifstream MyReadFile("filename.txt");

    // Use a while loop together with the getline() function to read the file line by line
    //while (getline (MyReadFile, myText)) {
      // Output the text from the file
      //cout << myText;
    //}

    // Close the file
    //MyReadFile.close();
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char* hello = "Hello from server\n";
 
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
 
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
 
    // Forcefully attaching socket to the port 8080
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
    readFiles(new_socket);
    writeFiles(new_socket, fileNames);
    formatFiles(fileNames);
    // closing the connected socket
    close(new_socket);
    // closing the listening socket
    close(server_fd);
    return 0;
}
