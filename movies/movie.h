/* struct for Movie information */

struct Movie *createMovie(char *currLine);
struct Movie *processFile(char *filePath);
void printMovie(struct Movie *aMovie);
void printMovieList(struct Movie *list);
void displayMoviesFromYear(int year, struct Movie *list);
void displayTopMovies(struct Movie *list);
void displayMoviesInLanguage(char *langptr, struct Movie *list);
void swap(struct Movie *a, struct Movie *b);
void bubbleSort(struct Movie *start);
int movieContainsLanguage(char *langptr, struct Movie *node);