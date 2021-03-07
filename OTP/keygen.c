#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

char chars[27] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";

int writekeys(int keylen);

int main(int argc, char* argv[]) {
  int random;
  int key;
  
  if (argc < 2){
    fprintf(stderr, "Please specify the size of the key file.\n");
    return 1;
  }

  //write the keys to stdout
  key = writekeys(atoi(argv[1]));
  return 0;
}

int writekeys(int keylen)
{
  //initialize random number generator seed
  srand(time(0));
  int key;
  
  for (key = 0; key < keylen; key++)
  {
        putchar(chars[rand() % 27]);
  }
  putchar('\n');
  return key;
}