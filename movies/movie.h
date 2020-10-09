/* struct for Movie information */

struct Movie *createMovie(char *currLine);
struct Movie *processFile(char *filePath);
void printMovie(struct Movie *aMovie);
void printMovieList(struct Movie *list);
void displayMoviesFromYear(int year, struct Movie *list);
void displayTopMovies();
void displayMoviesInLanguage(char *langptr);
void swap(struct Movie *a, struct Movie *b);
void bubbleSort(struct Movie *start);