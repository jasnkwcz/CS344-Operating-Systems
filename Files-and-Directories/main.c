#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "processFiles.h"

int main(void)
{
  //create a union to hold user responses
  union token
  {
    int *num;
    char *filename;
  };

  union token response;

  do
  {
    //display main menu
    printf("1: Select file to process\n2: Exit program\nSelect (1-2): ");
    scanf("%d", response.num);

    switch (*response.num)
    {
    case 1:
      printf("1: Largest file\n2: Smallest file\n3: Select file by name\nSelect (1-3): ");
      scanf("%d", response.num);

      switch (*response.num)
      {
      case 1:
        processLargest();
        break;
      case 2:
        processSmallest();
        break;
      case 3:
        //logic for selecting file by name
        printf("Please enter the filename (including extension): ");
        scanf("%s", response.filename);
        printf("Processing file: %s\n", response.filename);
        processByName(response.filename);
        break;
      default:
        printf("Invalid selection\n");
        break;
      }

      break;
    case 2:
      printf("Goodbye\n");
      exit(0);
    default:
      printf("Invalid selection\n");
      break;
    }
  } while (true);
  return 0;
}
