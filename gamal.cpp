#include <stdlib.h>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <string.h>
#include <cstring>
#include "gamal.h"
using namespace std;

unsigned long long gen_rand() {
  unsigned long long min = llroundl(pow(2, 31) + 1);
  unsigned long long max = llroundl(pow(2, 32));
  return llroundl(rand() % (max - min + 1) + min);
}

unsigned long long gcd(unsigned long long a, unsigned long long b) {
  if (a < b) {
    return gcd(b, a);
  } else if (a % b == 0) {
    return b;
  } else {
    return gcd(b, a % b);
  }
}

unsigned long long gen_key(unsigned long long q) {
  unsigned long long key = gen_rand();
  while (gcd(q, key) != 1) {
    key = gen_rand();
  }
  return key;
}

unsigned long long power(unsigned long long a, unsigned long long b, unsigned long long c) {
  unsigned long long x = 1;
  unsigned long long y = a;
  while (b > 0) {
    if (b % 2 != 0) {
      x = (x * y) % c;
    }
    y = (y * y) % c;
    b /= 2;
  }
  if (x == 0) {
    x = 1;
  }
  return x % c;
}

string encrypt(string msg, unsigned long long key) {
  string en_msg;
  for (int i = 0; i < strlen(msg.c_str()); i++) {
    en_msg += to_string(key * msg[i]);
    en_msg += "|";
  }
  return en_msg;
}

string decrypt(string en_msg, unsigned long long key) {
  string dr_msg;
  int currentIndex = 0;
  for (int i = 0; i < strlen(en_msg.c_str()); i++) {
    if (en_msg[i] == '|') {
      dr_msg += (char) (stoull(en_msg.substr(currentIndex, i - currentIndex)) / key);
      currentIndex = i + 1;
    }
  }
  return dr_msg;
}

string secret_encrypt(string msg, unsigned long long secretKey, unsigned long long q) {
  string en_msg;
  string en_msg2;
  for (int i = 0; i < strlen(msg.c_str()); i++) {
    en_msg += to_string((int) msg[i]);
  }
  en_msg = to_string(stoull(en_msg) % q);
  en_msg2 = to_string(power(stoull(en_msg), secretKey, q));
  return en_msg2;
}

string secret_final_encrypt(string msg, unsigned long long secretKey, unsigned long long q) {
  return to_string(power(stoull(msg), secretKey, q));
}

unsigned long long get_key(unsigned long long g, unsigned long long key, unsigned long long q) {
  return power(g, key, q);
}

string send_init_gamal(unsigned long long sharedQ, unsigned long long sharedG, unsigned long long sharedH) {
  return to_string(sharedQ) + "," + to_string(sharedG) + "," + to_string(sharedH);
}

string send_first_response(unsigned long long sharedH2, string enc_message) {
  return to_string(sharedH2) + "," + enc_message;
}

string send_second_response(string enc_message) {
  return enc_message;
}

unsigned long long gen_g() {
  unsigned long long min = llroundl(pow(2, 29) + 1);
  unsigned long long max = llroundl(pow(2, 30));
  return llroundl(rand() % (max - min + 1) + min);
}

string init_gamal(unsigned long long sharedQ, unsigned long long secretKey) {
  unsigned long long sharedG = gen_g();
  unsigned long long sharedH = power(sharedG, secretKey, sharedQ);
  return send_init_gamal(sharedQ, sharedG, sharedH);
}

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

size_t find_file_size(string fileName) {
  ifstream file;
  file.open(fileName, ios::in|ios::binary);
  file.ignore( std::numeric_limits<std::streamsize>::max() );
  std::streamsize length = file.gcount();
  file.clear();
  file.seekg( 0, std::ios_base::beg );
  file.close();
  return length;
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

bool areAllFilesDifferent(string serverFiles[5], string clientFiles[5]) {
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      if (compareFiles(serverFiles[i], clientFiles[j])) {
        return false;
      }
    }
  }
  return true;
}

void write_secret_keys(int new_socket, unsigned long long shared_q, unsigned long long shared_g, string gamalString) {
  string keysString = to_string(shared_q) + "," + to_string(shared_g) + "," + gamalString;
  send(new_socket, keysString.c_str(), strlen(keysString.c_str()), 0);
}