#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <fcntl.h>
#include "movie.h"
#include <unistd.h>

#define HIGH 99999
#define LOW 0
#define PREFIX "movies_"
#define EXT ".csv"

/*******************************************************************
 take an empty char array as input, create a pseudorandom directory name and store in input array
 *******************************************************************/
void makeDirname(char *dirname)
{
  srand(time(0));
  int random = (rand() % (HIGH - LOW + 1)) + LOW;
  char randomStr[6];
  sprintf(randomStr, "%d", random);
  randomStr[5] = '\0';
  strcpy(dirname, "sienkijo.movies.");
  strcat(dirname, randomStr);
  printf("Created new directory: %s\n", dirname);
  return;
}

/*******************************************************************
build a new directory and populate it with .txt files based on the movie file provided by <filename>
 *******************************************************************/
void buildDir(char *filename)
{
  char dirname[32];
  int fd;
  int wr;
  struct Movie *list;
  list = processFile(filename);
  makeDirname(dirname);
  if (mkdir(dirname, 0750) != 0)
  {
    printf("Error making directory. Error code %d\n", errno);
  };
  //open the new dir
  DIR *dir = opendir(dirname);
  if (dir == NULL)
  {
    printf("Error opening directory. Error number %d\n", errno);
  }

  struct Movie *curr = list;
  char file[32];
  chdir(dirname);
  while (curr != NULL)
  {
    strcpy(file, curr->year);
    strcat(file, ".txt");
    fd = open(file, O_RDWR | O_APPEND | O_CREAT, 0640);
    wr = write(fd, curr->title, strlen(curr->title));
    wr = write(fd, "\n", 1);
    fd = close(fd);
    curr = curr->next;
  }
  chdir("..");

  return;
}

/*******************************************************************
finds the largest file with the extension csv in the current directory whose name starts with the prefix movies_ and automatically process it.
In case of tie, pick any of the files with the extension csv starting with movies_ that have the largest size.
*******************************************************************/
void processLargest()
{
  //iterate through all inodes in the current directory, checking the metadata of each node to find largest file of type .csv that also has the prefix "movies_"
  //process that node using processFile()
  DIR *dir = opendir(".");
  struct dirent *file;
  struct dirent *max;
  struct stat dirstat;
  struct stat maxstat;
  char extension[5];
  while ((file = readdir(dir)) != NULL)
  {
    if (max == NULL)
    {
      max = file;
    }
    stat(file->d_name, &dirstat);
    stat(max->d_name, &maxstat);

    strcpy(extension, &file->d_name[strlen(file->d_name) - 4]); //check if .csv file
    if (strcmp(EXT, extension) == 0)
    {
      //then check if filename has prefix "movies_"
      if (strncmp(PREFIX, file->d_name, 7) == 0)
      {
        //check if file size is larger than the current max size
        if ((int)dirstat.st_size >= (int)maxstat.st_size)
        {
          max = file;
        }
      }
    }
  }
  buildDir(max->d_name);
  return;
}

/*******************************************************************
finds the smallest file with the extension csv in the current directory whose name starts with the prefix movies_ and automatically process it.
In case of tie, pick any of the files with the extension csv starting with movies_ that have the smallest size.
*******************************************************************/
void processSmallest()
{
  DIR *dir = opendir(".");
  struct dirent *file;
  struct dirent *min;
  struct stat dirstat;
  struct stat minstat;
  char extension[5];
  while ((file = readdir(dir)) != NULL)
  {
    stat(file->d_name, &dirstat);
    stat(min->d_name, &minstat);

    strcpy(extension, &file->d_name[strlen(file->d_name) - 4]); //check if .csv file
    if (strcmp(EXT, extension) == 0)
    {
      //then check if filename has prefix "movies_"
      if (strncmp(PREFIX, file->d_name, 7) == 0)
      {
        //check if file size is less than the current min size
        if ((int)dirstat.st_size <= (int)minstat.st_size)
        {
          min = file;
        }
      }
    }
  }
  buildDir(min->d_name);
  return;
}
