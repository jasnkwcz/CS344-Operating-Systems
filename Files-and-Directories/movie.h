/* struct for Movie information */
struct Movie
{
    char *title;
    char *year;
    char *languages;
    float rating;
    struct Movie *next;
};
struct Movie *createMovie(char *currLine);
struct Movie *processFile(char *filePath);
