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

void send_message(string msg, unsigned long long secretS1, unsigned long long secretS2, unsigned long long secretQ) {
  unsigned long long sharedQ = gen_rand();
  unsigned long long sharedG = gen_g();
  unsigned long long secretKeyElGamal = gen_key(sharedQ);
  unsigned long long sharedH = power(sharedG, secretKeyElGamal, sharedQ);
  
  cout<<"Reciever to sender: "<<endl;
  //cout<<"\tq used: "<<sharedQ<<endl;
  //cout<<"\tg used: "<<sharedG<<endl;
  cout<<send_init_gamal(sharedQ, sharedG, sharedH)<<endl;
  //cout<<"\tg^a used: "<<sharedH<<endl;
  
  string en_msg = secret_encrypt(msg, secretS1, secretQ);
  cout<<"Sender generates: "<<en_msg<<endl;
  unsigned long long secretKeyElGamal2 = gen_key(sharedQ);
  unsigned long long sharedH2 = power(sharedG, secretKeyElGamal2, sharedQ);
  unsigned long long sender_key = power(sharedH, secretKeyElGamal2, sharedQ);
  en_msg = encrypt(en_msg, sender_key);
  
  cout<<"Sender to reciever: "<<endl;
  cout<<send_first_response(sharedH2, en_msg);
  //cout<<"\tg^ak used: "<<sender_key<<endl;
  //cout<<"\tSecret Key: "<<secretKeyElGamal2<<endl;
  //cout<<"\tencrypted message: "<<en_msg<<endl;

  unsigned long long reciever_key = get_key(sharedH2, secretKeyElGamal, sharedQ);
  string dr_msg = decrypt(en_msg, reciever_key);
  cout<<"Decrypted Message: "<<dr_msg<<endl;
  dr_msg = secret_encrypt(dr_msg, secretS2, secretQ);
  cout<<"Reciever generates: "<<dr_msg<<endl;
  dr_msg = encrypt(dr_msg, reciever_key);
  
  cout<<"Reciever to sender: "<<endl;
  cout<<send_second_response(dr_msg)<<endl;
  //cout<<"\tEncrypted message: "<<dr_msg<<endl;
  cout<<"Final message: "<<decrypt(dr_msg, sender_key)<<endl;
}
