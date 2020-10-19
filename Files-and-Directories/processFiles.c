#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>



#define HIGH 99999
#define LOW 0




/*******************************************************************
Create a new directory and print a message with the name of the directory that has been created
The directory must be named your_onid.movies.random
where
random is a random number between 0 and 99999 (both numbers inclusive)
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
void processFile() {
  //generate random number [0..99999]
  srand(time(0));
  int random = (rand() %  (HIGH - LOW + 1)) + LOW; 
  printf("%d", random);
  char randomStr[5];
  itoa(random, randomStr);
  printf("%s", randomStr);
  //convert random to string
  //append random to sienkijo.movies to produce full dirname
  //create new dir with dirname using mkdir
  //open the new dir
  //reading from the input file:
  //while getline !null, parse the file line by line
  //for each movie, create/append the file for the movie's corresponding year (only need to write the movie title)
  //close dir

  return;
}




/*******************************************************************
finds the largest file with the extension csv in the current directory whose name starts with the prefix movies_ and automatically process it.
In case of tie, pick any of the files with the extension csv starting with movies_ that have the largest size.
*******************************************************************/
void processLargest() {
  //iterate through all inodes in the current directory, checking the metadata of each node to find largest file of type .csv that also has the prefix "movies_"
  //process that node using processFile()
  processFile();
  return;
}


/*******************************************************************
finds the smallest file with the extension csv in the current directory whose name starts with the prefix movies_ and automatically process it.
In case of tie, pick any of the files with the extension csv starting with movies_ that have the smallest size.
*******************************************************************/
void processSmallest(){
  //iterate through all inodes in the current directory, checking the metadata of each node to find smallest file of type .csv that also has the prefix "movies_"
  //process that node using processFile()
   processFile();
  return;
}

/*******************************************************************
asks the user to enter the name of a file.
The program checks if this file exists in the current directory. If the file is not found, the program should write an error message and again give the user the 3 choices about picking a file, i.e., don't go back to the main menu, but stay at the menu for picking a file.
*******************************************************************/
void processByName(char* filename) {
  //iterate over each inode to find a file with the given name
  //if no file is found, write an error message to std output
  //when file is found, process the file using processFile()
   processFile();
  return;
}