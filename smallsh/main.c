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
}

int
main(void)
{
    //assign custom signal handlers for SIGINT and SIGSTP

    while (true)
    {
        //allocate memory for a new command struct including char arrays, etc.
        //initialize variables
        //get user input
        getUserInput();
        //parse input
        //check if input is a comment or blank, if so, ignore
        //run the user command
        //clean up and destroy the command struct

        //vvvvONLY FOR DEBUGGING. REMOVE WHEN DONE **********************************************************************
        break;
        //^^^^ONLY FOR DEBUGGING. REMOVE WHEN DONE **********************************************************************
    }
    //clean up
    return;
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
int getUserInput()
{
    char prompt = ": ";
    write(STDOUT_FILENO, prompt, strlen(prompt);
    fflush();
    return 0;
}

/*
parseInput()
    if the user input is blank or begins with "#", then input is ignored and returns 0
    else parse the input by splitting into tokens, storing in 
*/

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
/*

/*
Built-in command: status
    runs in foreground only
    prints out either the exit status or the terminating signal of the last foreground process run by the shell
    If this command is run before any foreground command is run, then it should simply return the exit status 0.
    ignores the other built-in commands (cd and exit)
*/