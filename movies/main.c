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
    struct Movie *list = processFile(argv[1]);
    printMovieList(list);
    return EXIT_SUCCESS;
}
