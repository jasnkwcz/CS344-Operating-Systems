#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINES 50
#define MAXCHARS 1000
#define OUTLINESIZE 80

//a line containing only this string will signify that the end of input is reached
char* stop = "STOP\n";


//buffer for input thread to pass to line separator
char buff1[MAXLINES][MAXCHARS];

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
char buff2[MAXLINES][MAXCHARS];

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
//index for consumer to get from buff3
int getbuff3 = 0;
//index where producer will place into buff3
int countbuff3 = 0;
//buff3 mutex
pthread_mutex_t buff3mutex = PTHREAD_MUTEX_INITIALIZER;
//buff3 condition variable
pthread_cond_t buff3full = PTHREAD_COND_INITIALIZER;

//flag to signal the last consumer when the buffer is not going to receive any more data
int buff3end = -1;

/*
//put a line into the first buffer
*/
void putBuff1(char* buff)
{
  //lock the mutex
  pthread_mutex_lock(&buff1mutex);
  
  //place the item in the buffer at the buff1 index
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
void getBuff1(char* get)
{
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
  return;
}


/*
main function to run from the input thread
get input
*/
void* getInput(void* arg)
{
  //flag to control the input loop, set to 1 when the end of input is reached, 0 otherwise
  int flag = 0;
  //allocate an input buffer to read in the user's input
  char* inputbuff = (char*)calloc(MAXCHARS, sizeof(char));
  //while the eof flag has not been set:
    //read in a line of input from the user, store it in the input buffer
    //if STOP is entered, then set the end of file flag and return
    //otherwise, put the input line in buff1
  while (flag != 1) {
    int endinput = 0;
    read(STDIN_FILENO, inputbuff, MAXCHARS);
    inputbuff[MAXCHARS-1] = '\0';
    if(strcmp(inputbuff, stop) == 0)
    {
      flag = 1;
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
void getBuff2(char* get)
{
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
  return;
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
    char* temp = (char*)calloc(MAXCHARS + 1, sizeof(char));

    //get an item from buff1, store it in a temp
    getBuff1(temp);
    //in the temp, replace newline character with space
    char* ls = strchr(temp, '\n');
    strcpy(&temp[ls - temp]," ");
    if (strcmp(temp, "STOP ") == 0)
    {
      endflag = 1;
      //remove trailing space from stop line
      temp[ls - temp] = '\0';
    }
    //put the temp in buff2
    putBuff2(temp);
    free(temp);
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
    free(newstr);
    return;
}


/*
//place a line into the third buffer
*/
void putBuff3(char* buff)
{
  //lock the mutex
  pthread_mutex_lock(&buff3mutex);

  //concatenate the string with buff3
  strcat(buff3, buff);

  //increment the indexes for the number of items waiting and total items put into buff1
  countbuff3 += strlen(buff);

  pthread_cond_signal(&buff3full);

  if(strcmp(buff, "STOP") == 0){
    buff3end = 1;
  }

  //unlock the mutex
  pthread_mutex_unlock(&buff3mutex);
  return;
}


/*
main function to run from the replace variables thread
//replace variable instances
*/
void* replaceVars(void* arg)
{
  //initialize a flag to check if the end of the input has been reached
  int endflag = 0;
  char* find = "++";
  char* replace = "^";
  
  char* temp = (char*)calloc(MAXCHARS, sizeof(char));

  //until the end flag is set:
    //get an item from buff2, store it in temp
    //run search and replace algo on the temp
    //put the new string in buff3
  while (endflag != 1)
  {
    getBuff2(temp);
    if(strcmp(temp, "STOP") == 0){
      endflag = 1;
    }
    findAndReplace(&temp, find, replace);
    putBuff3(temp);
    memset(temp, 0, MAXCHARS);
  }
  free(temp);
 return NULL;
}


//get a line from the third buffer

void getBuff3(char* get)
{
  //lock the mutex
  pthread_mutex_lock(&buff3mutex);
  
  //check if there are 80 or more characters in the buffer
    //if no, check if the end of file flag has been set
      //if yes, then free "get" and return null
      //if no, then wait for more buffer contents
    //if yes, get them and return them normally
  while (countbuff3 < 80) {
    if (buff3end == 1)
    {
      strcpy(get, "");
      return;
    } else 
    {
      pthread_cond_wait(&buff3full, &buff3mutex);
    }
  }

  //get the item in buff3
  strncpy(get, &buff3[getbuff3], OUTLINESIZE);
  //increment the buffer pick index by 80
  getbuff3 += strlen(get);
  //decrement the number of remaining items to pick out of buff1
  countbuff3 -= strlen(get);
  //unlock the mutex

  pthread_mutex_unlock(&buff3mutex);
  //return the string grabbed from buff3
  return;
}


/*
main function to run from the output thread
write to standard output
*/
void* writeOut(void* arg)
{
  //holds the line to be printed
  char* line = (char*)calloc(OUTLINESIZE, sizeof(char));
;
  //flag to control the loop for writing output
  int endfile = 0;

  //keep printing the output until the end of file is detected
  while (endfile != 1)
  {
    getBuff3(line);
    if (strcmp(line, "\0") == 0)
    {
      endfile = 1;
      free(line);
      return NULL;
    } else
    {
      write(STDOUT_FILENO, line, 80);
      write(STDOUT_FILENO, "\n", 1);
    }
  }
  free(line);
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
