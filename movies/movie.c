#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "movie.h"

struct Movie
{
  char *title;
  int year;
  char *languages;
  float rating;
  struct Movie *next;
};

void printMovieList(struct Movie *list)
{
  while (list != NULL)
  {
    printMovie(list);
    list = list->next;
  }
}

void printMovie(struct Movie *aMovie)
{
  printf("%s, %s, %d, %.1f\n", aMovie->title,
         aMovie->languages,
         aMovie->year,
         aMovie->rating);
}

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
  int yeartoken = atoi(strtok_r(NULL, ",", &saveptr));
  currMovie->year = yeartoken;

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

void displayMoviesFromYear(int year, struct Movie *list)
{
  printf("Displaying movies from %d:\n", year);
  int count;
  struct Movie *curr = list;
  while (curr != NULL)
  {
    if (curr->year == year)
    {
      printf("%s\n", curr->title);
      ++count;
    }
    curr = curr->next;
  }
  if (count == 0)
  {
    printf("No available movies from %d\n", year);
  }
  return;
}

void displayTopMovies(struct Movie *list)
{
  printf("Displaying highest rated movies for each year:\n");
  //sort list of movies by year
  bubbleSort(list);
  //iterate through list, printing top movie for each year
  struct Movie *curr = list;
  struct Movie *max = curr;
  while (curr != NULL)
  {
    //check if the current node's year is different from the max
    //if yes, print the max, then set max = curr
    //otherwise, check if curr rating is higher than max rating
    //if yes, set max = curr
    //before next loop, move curr forward

<<<<<<< HEAD
    if (curr->year != max->year)
    {
      printf("%d - %.1f - %s\n", max->year, max->rating, max->title);
      max = curr;
      if (curr->next == NULL)
      {
        printf("%d - %.1f - %s\n", max->year, max->rating, max->title);
        break;
      }
    }
    else if (curr->rating > max->rating)
    {
=======
    if (curr->year != max->year) {
      printf("%d - %.1f - %s\n", max->year, max->rating, max->title);
      max = curr;
      if (curr->next == NULL) {
        printf("%d - %.1f - %s\n", max->year, max->rating, max->title);
        break;
      }
    } else if (curr->rating > max->rating) {
>>>>>>> 9e4afbcda8ea69710892dc4843fddcf66bd98cce
      max = curr;
    }
    curr = curr->next;
  }
  return;
}

void displayMoviesInLanguage(char *langptr, struct Movie *list)
{
  printf("Displaying all movies in %s:\n", langptr);
  struct Movie *curr = list;
  while (curr != NULL)
  {
    if (movieContainsLanguage(langptr, curr) == 1)
    {
      printf("%d - %s\n", curr->year, curr->title);
    }
    curr = curr->next;
  }
  return;
}

int movieContainsLanguage(char *langptr, struct Movie *node)
{
  //use a token to extract language strings from the node
  char str[100];
  strcpy(str, node->languages);
  char *saveptr;
  char *token = strtok_r(str, "[];", &saveptr);
  while (token != NULL)
  {
    if (strcmp(token, langptr) == 0)
    {
      return 1;
    }
    token = strtok_r(NULL, "[];", &saveptr);
  }
  return 0;
}
//******************
//*****************
//The following code was adapted from the bubble sort implementation found on https://www.geeksforgeeks.org/c-program-bubble-sort-linked-list/
/* Bubble sort the given linked list */
void bubbleSort(struct Movie *start)
{
  int swapped, i;
  struct Movie *ptr1;
  struct Movie *lptr = NULL;

  if (start == NULL)
    return;

  do
  {
    swapped = 0;
    ptr1 = start;

    while (ptr1->next != lptr)
    {
      if (ptr1->year < ptr1->next->year)
      {
        swap(ptr1, ptr1->next);
        swapped = 1;
      }
      ptr1 = ptr1->next;
    }
    lptr = ptr1;
  } while (swapped);
}

/* function to swap data of two nodes a and b*/
void swap(struct Movie *a, struct Movie *b)
{
  //copy data from a into a temp
  struct Movie *temp = malloc(sizeof(struct Movie));
  temp->title = a->title;
  temp->year = a->year;
  temp->languages = a->languages;
  temp->rating = a->rating;
  //copy data from b into a
  a->title = b->title;
  a->year = b->year;
  a->languages = b->languages;
  a->rating = b->rating;
  //copy data from temp into b
  b->title = temp->title;
  b->year = temp->year;
  b->languages = temp->languages;
  b->rating = temp->rating;

  free(temp);
}

//end of code adapted from bubble sort implementation
//******************
//*****************
