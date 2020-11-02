//Joshua Sienkiewicz
//Oregon State University
//CS 344 - Operating Systems

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>


#define MAXCHARS 2048
#define MAXARGS 512
#define VAREXP "$$"

struct Command
{
    char *cmd;
    char **args;
    char *inFile;
    char *outFile;
    char *bg;
};

void strreplace(char **string, const char *search, const char *replace);
int getUserInput(char **line);
struct Command *parseInput(char **line);
//char *varExpand(char *str, char *find, char *replace);

int main(void)
{
    //assign custom signal handlers for SIGINT and SIGSTP

    //initialize varaibles
    char *nline; //a command line
    char **nargv; //array of char pointers to hold the argument vector, whose first entry is the command itself

    //main command prompt loop
    while (true)
    {
        //get user input, store it in nline. if it's blank, ignore it and restart the prompt loop
        if (getUserInput(&nline) == 1) {
            continue;
        };

        //parse input into a new command
        struct Command *cmd = parseInput(&nline);

        //check if input is a comment or blank, if so, ignore
        if (strcmp(cmd->cmd, "#") == 0) {
            continue;
        }
        //run the user command
        //clean up and destroy the command struct
        //vvvvONLY FOR DEBUGGING. REMOVE WHEN DONE **********************************************************************
        break;
        //^^^^ONLY FOR DEBUGGING. REMOVE WHEN DONE **********************************************************************
    }
    //clean up
    return 0;
}


/*
handleSigint()

*/

/*
handleSigstp()
*/

/*
getUserInput()
    prompts the user with ": " to enter a command 
    stores the user's input in a string specified in main
*/
int getUserInput(char** line)
{
    //print prompt to stdout
    char*  prompt = ": ";
    *line = (char *)calloc(MAXCHARS + 1, sizeof(char));
    write(STDOUT_FILENO, prompt, strlen(prompt));
    //read a line from stdout up to length of MAXCHARS
    read(STDIN_FILENO, *line, MAXCHARS);
    //resize the line argument based on the contents read from stdin, then copy the input into the line variable
    *line = (char*)realloc(*line, strlen(*line));
    return strlen(*line);
}

/*
parseInput()
    if the user input is blank or begins with "#", then input is ignored and returns 0
    else parse the input by splitting into tokens, storing in 
*/
struct Command* parseInput(char** line) {
    struct Command *newCmd = (struct Command *)malloc(sizeof(struct Command));
    newCmd->args = (char **)calloc(MAXARGS, sizeof(char*));

    //initialize variables for strtok_r and a counter for the argument list
    char *token;
    char *saveptr;
    int nargc = 0;
    char *varex;

    //first token is the command
    token = strtok_r(*line, " ", &saveptr);
    newCmd->cmd = (char *)calloc(strlen(token) + 1, sizeof(char));
    strcpy(newCmd->cmd, token);

    //if the variable expression given in VAREXP is found in the command string, replace it with the shell process id
    varex = strstr(token, VAREXP);
    if (strcmp(varex, VAREXP) == 0)
    {
        pid_t p = getpid();
        char pstr[16];
        sprintf(pstr, "%d", p);
        printf("found a $$! We should replace it with %s\n", pstr);
    }

    //begin parsing the arguments for the command    
    while ((token = strtok_r(NULL, " \n", &saveptr))!= NULL)
    {
        //handle input file delimiter, immediately get the next token and store it as the command's infile
        if (strcmp(token, ">") == 0) {
            token = strtok_r(NULL, " ", &saveptr);
            newCmd->inFile = (char *)calloc(strlen(token) + 1, sizeof(char));
            strcpy(newCmd->inFile, token);
        }

        //handle the output file delimiter, immediately get the next token and store it as the command's outfile
        else if (strcmp(token, "<") == 0) {
            token = strtok_r(NULL, " ", &saveptr);
            newCmd->outFile = (char *)calloc(strlen(token) + 1, sizeof(char));
            strcpy(newCmd->outFile, token);
        }

        //handle commands running in the background
        else if (strcmp(token, "&") == 0) {
            printf("found an &\n");
            if ((token = strtok_r(NULL, " ", &saveptr)) == NULL)
            {
                newCmd->bg = (char*)calloc(2, sizeof(char));
                strcpy(newCmd->bg, "&");
            }
        }

        //handle everything else (command arguments)
        else {
            newCmd->args[nargc] = (char *)calloc(strlen(token) + 1, sizeof(char));
            strcpy(newCmd->args[nargc], token);
            printf("argument %d for the new command is %s\n", nargc, newCmd->args[nargc]);
            ++nargc;
        }
    }

    return newCmd;
}

/*
varExpand()
    handles variable expansion when a "$$" is found in a command
    takes the command string as input, replaces the "$$" with the current process ID
    replace command string with the new string
*/

void varExpand(char **string, const char *search, const char *replace)
{
    return;
}

/*
runCmd()
    runs the command entered by the user

*/

/*
Built-in command: cd
    runs in foreground only
    if no args are specified, changes to the directory pointed to by the HOME environment variable
    takes up to one arg: a relative OR absolute path to a directory to change to

*/

/*
Built-in command: exit
    runs in foreground only
    takes no arguments
    terminates all other currently running processes and jobs, then terminates the shell program
*/

/*
Built-in command: status
    runs in foreground only
    prints out either the exit status or the terminating signal of the last foreground process run by the shell
    If this command is run before any foreground command is run, then it should simply return the exit status 0.
    ignores the other built-in commands (cd and exit)
*/