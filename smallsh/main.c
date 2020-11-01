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

struct Command
{
    char *cmd;
    char **args;
    char *inFile;
    char *outFile;
    bool *run_bg;
};

int getUserInput(char** line);
struct Command *makeCmd();
char **parseInput(char *line);

int main(void)
{
    //assign custom signal handlers for SIGINT and SIGSTP

    //initialize varaibles
    char *nline; //a command line
    struct Command *newcmd = makeCmd(); //a command struct
    char **nargv;

    while (true)
    {
        //get user input, store it in nline
        if (getUserInput(&nline) == 1) {
            continue;
        };
        //parse input
        nargv = parseInput(nline);
        //check if input is a comment or blank, if so, ignore
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
makeCmd()
    allocates memory for all command struct members and returns a pointer to the new command struct
*/
struct Command *makeCmd() {
    struct Command *newCmd = (struct Command *)malloc(sizeof(struct Command));
    return newCmd;
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
char **parseInput(char* line) {
    
}


/*
varExp()
    handles variable expansion when a "$$" is found in a command
    takes the command string as input, replaces the "$$" with the current process ID
    replace command string with the new string
*/

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