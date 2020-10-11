// If you are not compiling with the gcc option --std=gnu99, then
// uncomment the following line or you might get a compiler warning
//#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "movie.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("You must provide the name of the file to process\n");
        printf("Example usage: ./Movies Movie_info1.txt\n");
        return EXIT_FAILURE;
    }

    int response;
    int year;
    char *langptr = (char *)calloc(20, sizeof(char));

    struct Movie *list = processFile(argv[1]);
    do
    {
        printf("Menu:\n1. Show movies released in a specific year\n2. Show highest rated movie for each year\n3. Show all movies released in a specific language\n4. Exit program\nPlease select from the above options: ");
        scanf("%d", &response);
        switch (response)
        {
        case 1:
            printf("Please enter a year: ");
            scanf("%d", &year);
            printf("\n");
            displayMoviesFromYear(year, list);
            printf("\n");
            break;
        case 2:
            displayTopMovies(list);
            break;
        case 3:
            printf("Please select a language: ");
            scanf("%s", langptr);
            printf("\n");
            displayMoviesInLanguage(langptr, list);
            printf("\n");
            break;
        case 4:
            printf("Goodbye\n");
            break;
        default:
            printf("Invalid selection\n\n");
        }
    } while (response != 4);
    free(langptr);
    return EXIT_SUCCESS;
}
