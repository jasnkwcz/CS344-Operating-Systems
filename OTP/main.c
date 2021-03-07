#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

#define CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZ "
#define BUFFSIZE 1024

int encryptString(char* plaintext, char* key, char* ciphertext);
int ctoi(char c);
char itoc(int i);


int main(void) {
  char plain[] = "HELLO WORLD";
  char key[] = "ILPWQB APX";
  char cipher[BUFFSIZE] = "";

  encryptString(plain, key, cipher);
  printf("%s\n", cipher);
  return 0;
}

int encryptString(char* plaintext, char* key, char* ciphertext)
{
  int size = strlen(plaintext);
  memset(ciphertext, '\0', BUFFSIZE);
  char cipher_char;
  int cipher_int;
  char plain_char;
  int plain_int;
  char key_char;
  int key_int;


  for (int i = 0; i < size; i++)
  {
    plain_char = plaintext[i];
    plain_int = ctoi(plain_char);

    key_char = key[i];
    key_int = ctoi(key_char);

    cipher_int = (key_int + plain_int) % 27;
    cipher_char = itoc(cipher_int);
  }
  return 0;
}

//convert a character c into its corresponding integer index value in CHARS
int ctoi(char c)
{
  char* ptr = strchr(CHARS, c);
  int index = ptr - CHARS;
  return index;
}


//convert an integer into its corresponding character value in CHARS
char itoc(int i)
{
  return(CHARS[i]);
}