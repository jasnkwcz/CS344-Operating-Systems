//Joshua Sienkiewicz
//Oregon State University
//CS 344 - Operating Systems
//Project 3 - Smallsh

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

void findAndReplace(char **str, char *search, char *replace);
int getUserInput(char **line);
struct Command *parseInput(char **line);
void runCmd(struct Command *cmd);
char* cd_builtin(struct Command *cmd);
void exit_builtin();
void status_builtin();

int main(void)
{
    //assign custom signal handlers for SIGINT and SIGSTP

    //initialize varaibles
    char *nline; //a command line
    char **nargv; //array of char pointers to hold the argument vector, whose first entry is the command itself
    setenv("PWD", getenv("HOME"), 1);

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
        runCmd(cmd);
        //clean up and destroy the command struct
        //vvvvONLY FOR DEBUGGING. REMOVE WHEN DONE **********************************************************************
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
    token = strtok_r(*line, " \n", &saveptr);
    newCmd->cmd = (char *)calloc(strlen(token) + 1, sizeof(char));
    strcpy(newCmd->cmd, token);

    //if the variable expression given in VAREXP is found in the command string, replace it with the shell process id
    varex = strstr(token, VAREXP);
    if (varex != NULL)
    {
        pid_t p = getpid();
        char pstr[16];
        sprintf(pstr, "%d", p);
        findAndReplace(&token, VAREXP, pstr);
        free(newCmd->cmd);
        newCmd->cmd = (char *)calloc(strlen(token) + 1, sizeof(char));
        strcpy(newCmd->cmd, token);
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

void findAndReplace(char **str, char *search, char *replace)
{
    //allocate a buffer to hold the expanded string
    char *newstr = (char *)calloc(MAXCHARS, sizeof(char));

    //set a temp to traverse the string to search
    char *temp = *str;

    while ((temp = strstr(temp,search)))
    {
        strncpy(newstr, *str, temp - *str);
        newstr[temp - *str] = '\0';
        strcat(newstr, replace);
        strcat(newstr, temp + strlen(search));
        strcpy(*str, newstr);
    }

    **str = *newstr;

    return;
}

/*
void runCmd(struct Command *cmd)
    runs the command entered by the user

*/
void runCmd(struct Command *cmd)
{

    if (strcmp(cmd->cmd, "cd") == 0)
    {
        //change the PWD to the directory returned by the cd builtin
        char *newdir = cd_builtin(cmd);
        setenv("PWD", newdir, 1);
        char buffer[MAXCHARS];
        getcwd(buffer, MAXCHARS);
        printf("The current working directory is: %s\n", buffer);
    }

    else if (strcmp(cmd->cmd, "status") == 0)
    {
        status_builtin();
    }

    else if (strcmp(cmd->cmd, "exit") == 0)
    {
        exit_builtin();
    }

    else
    {
        printf("Executing external command %s\n", cmd->cmd);
        externalCmd(cmd);
    }
}

/*
void externalCmd()
    runs a non-built-in command by forking a child process and using exec() function

*/
void externalCmd(struct Command *cmd)
{
    pid_t cpid = fork();
    switch(cpid) 
    {
        case -1:
            perror("%s failed to execute\n", cmd->cmd);
            exit(1);
            break;
        case 0:

        default:
            //handle background commands
            if (strcmp(cmd->bg, "&") == 0)
            {
                printf("Command will run in background.\n")
            }

            //handle foreground commands
            else
            {
                printf("Command will run in foreground.\n")
            }

    }
}

/*
Built-in command: cd_builtin()
    runs in foreground only
    if no args are specified, changes to the directory pointed to by the HOME environment variable
    takes up to one arg: a relative OR absolute path to a directory to change to

*/
char* cd_builtin(struct Command *cmd)
{
    //handle the case where there is a directory path specified, change to specified directory
    if (cmd->args[0] != NULL) {
        //if cd is successful (path found), chdir returns 0
        if (chdir(cmd->args[0]) == 0)
        {
            printf("Changed current working directory to: %s\n", cmd->args[0]);
            return (getenv("PWD"));
        }

        else
        {
            printf("Directory not found.\n");
            return (getenv("PWD"));
        }
    }

    //case where no arguments provided to cd, set current directory to home environment variable value
    else {
        chdir(getenv("HOME"));
        return (getenv("PWD"));
    }
}

/*
Built-in command: exit_builtin()
    runs in foreground only
    takes no arguments
    terminates all other currently running processes and jobs, then terminates the shell program
*/
void exit_builtin()
{
    kill(0, SIGKILL);
}

/*
Built-in command: status_builtin()
    runs in foreground only
    prints out either the exit status or the terminating signal of the last foreground process run by the shell
    If this command is run before any foreground command is run, then it should simply return the exit status 0.
    ignores the other built-in commands (cd and exit)
*/
void status_builtin()
{
    pid_t cpid;
    int cpstatus;
    waitpid(-1, &cpstatus, 0);
    //the code below was adapted  from the reading material in the "process API - monitoring child processes" section
    if(WIFEXITED(cpstatus)){
      printf("Child process { %d } exited normally with status %d\n", cpid, WEXITSTATUS(cpstatus));
    } else{
      printf("Child process { %d } exited abnormally due to recieving signal %d\n", cpid, WTERMSIG(cpstatus));
    }
}