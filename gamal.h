#include <string.h>
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
void send_message(string msg, unsigned long long secretS1, unsigned long long secretS2, unsigned long long secretQ);
