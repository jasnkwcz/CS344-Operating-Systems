#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "processFiles.h"
#include <unistd.h>

//*********************************
//The following code is adapted from the starter code provided in the course repl.it file
//*********************************

struct Movie *processFile(char *filePath)
{
  // Open the specified file for reading only
  FILE *MovieFile = fopen(filePath, "r");

  char *currLine = NULL;
  size_t len = 0;
  ssize_t nread;
  char *token;

  int count = 0;

  // The head of the linked list
  struct Movie *head = NULL;
  // The tail of the linked list
  struct Movie *tail = NULL;

  // Read the file line by line
  while ((nread = getline(&currLine, &len, MovieFile)) != -1)
  {
    //ignore the column headers in the first line
    if (count == 0)
    {
      getline(&currLine, &len, MovieFile);
      ++count;
      continue;
    }
    // Get a new Movie node corresponding to the current line
    struct Movie *newNode = createMovie(currLine);
    ++count;
    // Is this the first node in the linked list?
    if (head == NULL)
    {
      // This is the first node in the linked link
      // Set the head and the tail to this node
      head = newNode;
      tail = newNode;
    }
    else
    {
      // This is not the first node.
      // Add this node to the list and advance the tail
      tail->next = newNode;
      tail = newNode;
    }
  }
  free(currLine);
  fclose(MovieFile);
  printf("Processed file %s and parsed data for %d movies.\n", filePath, count);
  return head;
}

struct Movie *createMovie(char *currLine)
{
  struct Movie *currMovie = malloc(sizeof(struct Movie));

  // For use with strtok_r
  char *saveptr;

  // The first token is the title
  char *token = strtok_r(currLine, ",", &saveptr);
  currMovie->title = calloc(strlen(token) + 1, sizeof(char));
  strcpy(currMovie->title, token);

  // The next token is the year
  token = strtok_r(NULL, ",", &saveptr);
  currMovie->year = calloc(strlen(token) + 1, sizeof(char));
  strcpy(currMovie->year, token);

  // The next token is the languages
  token = strtok_r(NULL, ",", &saveptr);
  currMovie->languages = calloc(strlen(token) + 1, sizeof(char));
  strcpy(currMovie->languages, token);

  // The last token is the rating
  float ratingtoken = strtof(strtok_r(NULL, "\n", &saveptr), NULL);
  currMovie->rating = ratingtoken;

  // Set the next node to NULL in the newly created Movie entry
  currMovie->next = NULL;

  return currMovie;
}
//*********************************
//End of code adapted from repl.it
//*********************************
