#include <string.h>
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

string secret_final_encrypt(string msg, unsigned long long secretKey, unsigned long long q);
unsigned long long gen_rand();
unsigned long long gcd(unsigned long long a, unsigned long long b);
unsigned long long gen_key(unsigned long long q);
unsigned long long power(unsigned long long a, unsigned long long b, unsigned long long c);
string encrypt(string msg, unsigned long long key);
string decrypt(string en_msg, unsigned long long key);
string secret_encrypt(string msg, unsigned long long secretKey, unsigned long long q);
unsigned long long get_key(unsigned long long g, unsigned long long key, unsigned long long q);
string send_init_gamal(unsigned long long sharedQ, unsigned long long sharedG, unsigned long long sharedH);
string send_first_response(unsigned long long sharedH2, string enc_message);
unsigned long long gen_g();
string init_gamal(unsigned long long sharedQ, unsigned long long secretKey);
int is_regular_file(const char *path);
string* getFileNames();
string find_shared_string(char buffer[], int numOfComma);
size_t find_file_size(string fileName);
bool compareFiles(string fileName1, string fileName2);
bool areAllFilesDifferent(string serverFiles[5], string clientFiles[5]);
void write_secret_keys(int new_socket, unsigned long long shared_q, unsigned long long shared_g, string gamalString);