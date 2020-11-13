#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINES 50
#define MAXCHARS 1000
#define OUTLINESIZE 80

//prompt the user to enter more
char* prompt = ": ";
//a line containing only this string will signify that the end of input is reached
char* stop = "STOP\n";


//buffer for input thread to pass to line separator
char* buff1[MAXLINES];

//index where consumer will get from buff1
int getbuff1 = 0;
//index where producer will place into buff1
int putbuff1 = 0;
//count the number of items waiting in buff1
int countbuff1 = 0;
//buff1 mutex
pthread_mutex_t buff1mutex = PTHREAD_MUTEX_INITIALIZER;
//buff1 condition variable
pthread_cond_t buff1cond = PTHREAD_COND_INITIALIZER;


//buffer for line separator thread to pass to variable replacement
char* buff2[MAXLINES];

//index where consumer will get from buff2
int getbuff2 = 0;
//index where producer will place into buff2
int putbuff2 = 0;
//count the number of items waiting in buff2
int countbuff2 = 0;
//buff1 mutex
pthread_mutex_t buff2mutex = PTHREAD_MUTEX_INITIALIZER;
//buff1 condition variable
pthread_cond_t buff2cond = PTHREAD_COND_INITIALIZER;


//buffer for variable replacement thread to pass to output
char buff3[MAXLINES * MAXCHARS];

//index where producer will place into buff2
int countbuff3 = 0;
//buff1 mutex
pthread_mutex_t buff3mutex = PTHREAD_MUTEX_INITIALIZER;
//buff1 condition variable
pthread_cond_t buff3cond = PTHREAD_COND_INITIALIZER;


/*
//put a line into the first buffer
*/
void putBuff1(char* buff)
{
  //lock the mutex
  pthread_mutex_lock(&buff1mutex);
  
  //place the item in the buffer at the buff1 index
  buff1[putbuff1] = (char*)calloc(strlen(buff), sizeof(char));
  strcpy(buff1[putbuff1],buff);

  //increment the indexes for the number of items waiting and total items put into buff1
  ++putbuff1;
  ++countbuff1;
  //signal that buff1 is no longer empty
  pthread_cond_signal(&buff1cond);
  //unlock the mutex
  pthread_mutex_unlock(&buff1mutex);
  return;
}


/*
//get a line from the first buffer
*/
char* getBuff1(void)
{
  char* get = (char*)calloc(MAXCHARS, sizeof(char));
  //lock the mutex
  pthread_mutex_lock(&buff1mutex);
  while(countbuff1 == 0)
  {
    pthread_cond_wait(&buff1cond ,&buff1mutex);
  }
  //get the item in buff1 at the replace buffer index
  strcpy(get, buff1[getbuff1]);
  //increment the buffer pick index
  ++getbuff1;
  //decrement the number of remaining items to pick out of buff1
  --countbuff1;
  //unlock the mutex
  pthread_mutex_unlock(&buff1mutex);
  return get;
}


/*
main function to run from the input thread
get input
*/
void* getInput(void* arg)
{
  //flag to control the input loop, set to 1 when the end of input is reached, 0 otherwise
  int eofflag = 0;
  //allocate an input buffer to read in the user's input
  char* inputbuff = (char*)calloc(MAXCHARS, sizeof(char));
  //while the eof flag has not been set:
    //read in a line of input from the user, store it in the input buffer
    //if STOP is entered, then set the end of file flag and return
    //otherwise, put the input line in buff1
  while (eofflag != 1) {
    int endinput = 0;
    write(STDOUT_FILENO, prompt, strlen(prompt));
    read(STDIN_FILENO, inputbuff, MAXCHARS-1);
    inputbuff[MAXCHARS-1] = '\0';
    if(strcmp(inputbuff, stop) == 0)
    {
      eofflag = 1;
    }
    putBuff1(inputbuff);
    memset(inputbuff, 0, MAXCHARS);
    
  }
  free(inputbuff);
  return NULL;
}

/*
place a line into the second buffer
*/
void putBuff2(char* buff)
{
  //lock the mutex
  pthread_mutex_lock(&buff2mutex);
  
  //place the item in the buffer at the buff1 index
  buff2[putbuff2] = (char*)calloc(strlen(buff), sizeof(char));
  strcpy(buff2[putbuff2],buff);

  //increment the indexes for the number of items waiting and total items put into buff1
  ++putbuff2;
  ++countbuff2;
  //signal that buff2 is no longer empty
  pthread_cond_signal(&buff2cond);
  //unlock the mutex
  pthread_mutex_unlock(&buff2mutex);
  return;
}


/*
//get a line from the second buffer
*/
char* getBuff2(void)
{
  char* get = (char*)calloc(MAXCHARS, sizeof(char));
  //lock the mutex
  pthread_mutex_lock(&buff2mutex);
  while(countbuff2 == 0)
  {
    pthread_cond_wait(&buff2cond ,&buff2mutex);
  }
  //get the item in buff1 at the replace buffer index
  strcpy(get, buff2[getbuff2]);

  //increment the buffer pick index
  ++getbuff2;
  //decrement the number of remaining items to pick out of buff2
  --countbuff2;
  //unlock the mutex
  pthread_mutex_unlock(&buff2mutex);
  return get;
}


/*
main function to run from the line separator replacement thread
replace line separators
*/
void* replaceNewline(void* arg)
{
  //initialize a flag to detect when the end of input is reached
  int endflag = 0;
  //while the end of input flag is not set:
  while (endflag != 1)
  { 
    //get an item from buff1, store it in a temp
    char* temp = getBuff1();
    //in the temp, replace newline character with space
    char* ls = strchr(temp, '\n');
    temp[ls - temp] = ' ';
    //put the temp in buff2
    putBuff2(temp);
    if (strcmp(temp, "STOP ") == 0)
    {
      endflag = 1;
    }
  }
  return NULL;
}


void findAndReplace(char **str, char *search, char *replace)
{
    //allocate a buffer to hold the expanded string
    char *newstr = (char *)calloc(MAXCHARS, sizeof(char));

    //set a temp to traverse the string to search
    char *temp = *str;

    while ((temp = strstr(temp,search)))
    {
        strncpy(newstr, *str, temp - *str);
        newstr[temp - *str] = '\0';
        strcat(newstr, replace);
        strcat(newstr, temp + strlen(search));
        strcpy(*str, newstr);
    }

    return;
}


/*
//place a line into the third buffer
*/
void putBuff3(char* buff)
{
  //lock the mutex
  pthread_mutex_lock(&buff3mutex);
  //place the item in the buffer at the buff1 index
  strcat(buff3, buff);

  //increment the indexes for the number of items waiting and total items put into buff1
  countbuff3 += strlen(buff);
  //signal that buff3 is no longer empty, only if 80 or more characters are waiting to be output
  if (countbuff3 >= 80) 
  {
    pthread_cond_signal(&buff3cond);
  }
  //unlock the mutex
  pthread_mutex_unlock(&buff3mutex);
  return;
}

/*
//get a line from the third buffer

char* getBuff3(void)
{
  char* get = (char*)calloc(MAXCHARS, sizeof(char));
  //lock the mutex
  pthread_mutex_lock(&buff3mutex);
  while(countbuff3 == 0)
  {
    pthread_cond_wait(&buff2cond ,&buff2mutex);
  }
  //get the item in buff1 at the replace buffer index
  strcpy(get, buff2[getbuff2]);
  //increment the buffer pick index
  ++getbuff2;
  //decrement the number of remaining items to pick out of buff1
  --countbuff2;
  //unlock the mutex
  pthread_mutex_unlock(&buff2mutex);
  return get;
 return NULL;
}
*/



/*
main function to run from the replace variables thread
//replace variable instances
*/
void* replaceVars(void* arg)
{
  char * temp;
  //initialize a flag to check if the end of the input has been reached
  int endflag = 0;
  char* find = "++";
  char* replace = "^";
  //until the end flag is set:
    //get an item from buff2, store it in temp
    //run search and replace algo on the temp
    //put the new string in buff3
  while (endflag != 1)
  {
    temp = getBuff2();
    if(strcmp(temp, "STOP ") == 0){
      endflag = 1;
      return NULL;
    }
    findAndReplace(&temp, find, replace);
    putBuff3(temp);
    memset(temp, 0, MAXCHARS);
  }
 return NULL;
}

/*
main function to run from the output thread
write to standard output
*/
void* writeOut(void* arg)
{
  //while there are still characters waiting to be printed:
    //check if there are 80 chars available to print
    //if no, wait
    //if yes, print exactly 80 characters followed by a newline

 return NULL;
}


int main(void) {
  //create separate threads for input, line separator removal, variable replacement, and output 
  pthread_t inthread, lsthread, varthread, outthread;

  //call threads to execute
  pthread_create(&inthread, NULL, getInput, NULL);
  pthread_create(&lsthread, NULL, replaceNewline, NULL);
  pthread_create(&varthread, NULL, replaceVars, NULL);
  pthread_create(&outthread, NULL, writeOut, NULL);

  //join threads
  pthread_join(inthread, NULL);
  pthread_join(lsthread ,NULL);
  pthread_join(varthread ,NULL);
  pthread_join(outthread ,NULL);

  return 0;
}

