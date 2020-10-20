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
Create a new directory and print a message with the name of the directory that has been created
The directory must be named your_onid.movies.random
where random is a random number between 0 and 99999 (both numbers inclusive)
your_onid is your ONID
The permissions of the directory must be set to rwxr-x---
i.e., the owner has read, write and execute permissions, and group has read and execute permission to the directory.
Parse data in the chosen file to find out the movies released in each year
In the new directory, create one file for each year in which at least one movie was released
The permissions on these files must be set to rw-r-----
i.e., the owner can read and write to the file, while group can only read the file.
The file must be named YYYY.txt where YYYY is the 4 digit integer value for the year.
E.g., the file for movies released in 2018 must be named 2018.txt
Within the file for a year, write the titles of all the movies released in that year, one on each line
E.g., if two movies Avengers: Infinity War and Mary Queen of Scots where released in 2018, then the file 2018.txt will have two lines with each of the two titles on one line each.
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
    printf("Error making directory. Error code %d", errno);
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
    wr = write(fd, curr->title, strlen(curr->title) + 1);
    wr = write(fd, "\n", 2);
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
  buildDir("hello.csv");
  return;
}

/*******************************************************************
finds the smallest file with the extension csv in the current directory whose name starts with the prefix movies_ and automatically process it.
In case of tie, pick any of the files with the extension csv starting with movies_ that have the smallest size.
*******************************************************************/
void processSmallest()
{
  //iterate through all inodes in the current directory, checking the metadata of each node to find smallest file of type .csv that also has the prefix "movies_"
  //process that node using processFile()
  buildDir("hello.csv");
  return;
}

/*******************************************************************
asks the user to enter the name of a file.
The program checks if this file exists in the current directory. If the file is not found, the program should write an error message and again give the user the 3 choices about picking a file, i.e., don't go back to the main menu, but stay at the menu for picking a file.
*******************************************************************/
void processByName(char *filename)
{
  //iterate over each inode to find a file with the given name
  //if no file is found, write an error message to std output
  //when file is found, process the file using processFile()
  buildDir(filename);
  return;
}