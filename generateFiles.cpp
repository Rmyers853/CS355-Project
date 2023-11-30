#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <sstream>
using namespace std;

int getFileLength(string fileName) {
  ifstream in_file(fileName, ios::binary);
  in_file.seekg(0, ios::end);
  return in_file.tellg();
}

int main() {
  for (int i = 1; i < 11; i++) {
    string fileName = "Alice_file" + to_string(i) + ".txt";
    if (i > 5) {
      fileName = "Bob_file" + to_string(i-5) + ".txt";
    }
    int file_size = getFileLength(fileName);
    int previous_file_size = file_size;
    cout<<"Size of the file is"<<" "<< file_size<<" "<<"bytes"<<endl;
    ofstream MyFile;
    MyFile.open(fileName, ios::app);
    char randomChar = '!';
    unsigned seed= time(0);
    srand(seed);
    cout<<randomChar<<endl;
    while (file_size < 500000000) {
      for (int i = 0; i < 1000; i++) {
        randomChar = '!' + rand()%93;
        MyFile << randomChar;
      }
      file_size = getFileLength(fileName);
      if (previous_file_size != file_size) {
        cout<<"Size of the file is"<<" "<< file_size<<" "<<"bytes"<<endl;
        previous_file_size = file_size;
      }
    }
    MyFile.close();
  }
}
