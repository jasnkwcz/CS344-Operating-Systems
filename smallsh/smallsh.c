//Joshua Sienkiewicz
//Oregon State University
//CS 344 - Operating Systems
//Project 3 - Smallsh

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>


#define MAXCHARS 2048
#define MAXARGS 512
#define IOFILE 256
#define PIDVAR "$$"

struct Command
{
    char line[MAXCHARS];
    char* cmd;
    char* nargv[MAXARGS];
    char* infile;
    char* outfile;
    int nargc;
    int bg;
};


void readCmd(char* input);
void clearCmd(struct Command* cmd);
void pid_replace(char* cmd, const char* find, const char* replace);
void parseCmd(struct Command* cmd);
void displayCmd(struct Command *cmd);


int main() {
    //initialize relevant members of a new command struct
    struct Command newCmd;
    bool sh = true;
    pid_t ppid = getpid();

    while (sh == true)
    {
        //clear out the command struct
        clearCmd(&newCmd);
        //get command from standard input
        readCmd(newCmd.line);
        fflush(stdout);
        //if the command starts with '#', ignore it
        if (newCmd.line[0] == '#' || strcmp(newCmd.line, "") == 0)
        {
            continue;
        }
        //parse the command, expand variables
        char ppidstr[8];
        sprintf(ppidstr, "%d", ppid);
        pid_replace(newCmd.line, PIDVAR, ppidstr);
        parseCmd(&newCmd);

        //check if the command is built-in (exit, status, cd) and run

        //if command is external, execute using execvp

        //clean up command struct 
        displayCmd(&newCmd);
        fflush(stdout);
    }

    return(0);
}

/********************************************************************
 * void readCmd(char* line)
 * 
 * reads a user's command line from standard input and stores it in the
 * char array passed as 'line'
*********************************************************************/
void readCmd(char* line)
{
    char input[MAXCHARS];

    printf(": ");
    fflush(stdout);
    fgets(input, MAXCHARS, stdin);
    char* nline = strstr(input, "\n");
    nline[strcspn(nline, "\n")] = 0;
    strcpy(line, input);
    return;
}


/********************************************************************
 * void clearCmd(struct Command* cmd)
 * 
 * initialize all data members of a command struct so that it can be
 * used/reused during a given iteration of the main loop
*********************************************************************/
void clearCmd(struct Command* cmd)
{
    //clear line
    memset(cmd->line, 0, MAXCHARS);
    //clear cmd
    cmd->cmd = NULL;
    //clear nargv[]
    int i = 0;
    while (cmd->nargv[i] != NULL && i <= cmd->nargc)
    {
        cmd->nargv[i] = NULL;
        ++i;
    }

    //clear infile
    if (cmd->infile != NULL)
    {
        cmd->infile = NULL; 
    }
    
    //clear outfile
    if (cmd->outfile != NULL)
    {
        cmd->outfile = NULL;
    }
    //reset nargc and bg to 0
    cmd->nargc = 0;
    cmd->bg = 0;
    return;
}

/********************************************************************
 * void pid_replace(char* line, const char* find, const char* replace)
 * 
 * performs variable expansion on the string 'line'
 * takes a command line as input, replaces all occurrances of the variable'find' in the 
 * original string with the string passed as 'replace'
*********************************************************************/
void pid_replace(char* line, const char* find, const char* replace)
{
    //allocate a buffer to hold the expanded string
    char buff[MAXCHARS];
    //set an insertion point in the buffer
    char* i = &buff[0];
    //create a temporary constant copy of the command string, which points to where we will copy from into the buffer
    const char* temp = line;
    //get the length of the 'find' string
    int f = strlen(find);
    //get the length of the 'replace' string
    int r = strlen(replace);
    //pointer to check if any remaining variables left in the string
    const char* p;

    while(true)
    {
        p = strstr(temp, find);
        //in case the last instance of the variable has been found, copy the rest of the target string into the buffer and stop
        if (p == NULL)
        {
            strcpy(i, temp);
            break;
        }
        //copy temp up until the variable into the buffer
        memcpy(i, temp, p - temp);
        //update the insert point
        i += p - temp;
        //copy replacement string into buffer
        memcpy(i, replace, r);
        i += r;
        //update temp pointer
        temp = p + f;
    }
    //replace the command string with the new expanded string
    strcpy(line, buff);
    return;
}

/********************************************************************
 * void parseCmd(struct Command* newCmd)
 * 
 * parse a command line into tokens and store them in a command struct

*********************************************************************/
void parseCmd(struct Command* newCmd)
{
    //initialize variables to tokenize command line string
    char *token;
    char *saveptr;

    //first token is the command, save it as both the command and the first entry in args[]
    token = strtok_r(newCmd->line, " ", &saveptr);
    newCmd->nargv[0] = strdup(token);
    newCmd->cmd = strdup(token);
    ++newCmd->nargc;

    while ((token = strtok_r(NULL, " ", &saveptr))!= NULL)
    {
        //handle input file delimiter, immediately get the next token and store it as the command's infile
        if (strcmp(token, "<") == 0) 
        {
            token = strtok_r(NULL, " ", &saveptr);
            newCmd->infile = strdup(token);
        }

        //handle the output file delimiter, immediately get the next token and store it as the command's outfile
        else if (strcmp(token, ">") == 0) 
        {
            token = strtok_r(NULL, " ", &saveptr);
            newCmd->outfile = strdup(token);
        }

        //handle commands running in the background
        else if (strcmp(token, "&") == 0) 
        {
            if ((token = strtok_r(NULL, " ", &saveptr)) == NULL)
            {
                newCmd->bg = 1;
            }
        }

        //handle everything else (command arguments)
        else 
        {
            //if not, just add the argument to the argument list
                newCmd->nargv[newCmd->nargc] = strdup(token);
                ++(newCmd->nargc);
        }
        newCmd->nargv[newCmd->nargc] = NULL;
        printf("nargc is now: %d\n", newCmd->nargc);
    }
    return;
}


/********************************************************************
 * void displayCmd(struct Command *cmd)
 * 
 * used for debugging only
 * displays the command, argument list, input/output files, and background status
*********************************************************************/
void displayCmd(struct Command *cmd)
{
    printf("Command: %s\n", cmd->cmd);
    printf("Arguments: \n");
    for (int i = 0; i < cmd->nargc; i++)
    {
        printf("%s\n", cmd->nargv[i]);
    }
    if (cmd->infile != NULL)
    {
        printf("Input file is: %s\n", cmd->infile);
    }
    if (cmd->outfile != NULL)
    {
        printf("Output file is: %s\n", cmd->outfile);
    }
    if (cmd->bg == 1)
    {
        printf("Command will run in background.\n");
    }
}

void runCmd(struct Command *cmd)
{
    
    if (strcmp(cmd->cmd, "cd") == 0)
    {
        /*
        //change the PWD to the directory returned by the cd builtin
        char *newdir = cd_builtin(cmd);
        setenv("PWD", newdir, 1);
        char buffer[MAXCHARS];
        getcwd(buffer, MAXCHARS);
        printf("Current directory: %s\n", buffer);
        fflush(stdout);
        */
       printf("Running command: %s\n", cmd->cmd);
    }

    else if (strcmp(cmd->cmd, "status") == 0)
    {
        //status_builtin(fg_cpid);
        printf("Running command: %s\n", cmd->cmd);
    }

    else if (strcmp(cmd->cmd, "exit") == 0)
    {
        //exit_builtin();
        printf("Running command: %s\n", cmd->cmd);
    }

    else
    {
        //externalCmd(cmd);
        printf("Running command: %s\n", cmd->cmd);
    }
    return;
}