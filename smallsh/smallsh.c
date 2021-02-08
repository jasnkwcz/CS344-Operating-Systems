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
#define MAXPROCS 128
#define ARGLEN 64
#define PIDVAR "$$"

struct Command
{
    char line[MAXCHARS];
    char cmd[ARGLEN];
    char nargv[MAXARGS][ARGLEN];
    char infile[ARGLEN];
    char outfile[ARGLEN];
    int nargc;
    int bg;
};


void readCmd(char* input);
void initCmd(struct Command* cmd);
void pid_replace(char* cmd, const char* find, const char* replace);
void parseCmd(struct Command* cmd);
void displayCmd(struct Command *cmd);
void handle_SIGTSTP(int sig);

bool allow_bg = true;

int main() 
{
    struct Command newCmd;
    bool sh = true;
    pid_t ppid = getpid();
    pid_t cpid = 0;
    pid_t bg_procs[MAXPROCS];
    int bg_procs_index = 0;
    pid_t fg_cpid = 0;



    //assign custom signal handler to SIGTSTP
    struct sigaction SIGTSTP_action = {0};
    SIGTSTP_action.sa_handler = handle_SIGTSTP;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = 0;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

    while (sh == true)
    {
        //assign signal handler to ignore SIGINT (signal handling code taken from Exploration: "Signal handling API")
        struct sigaction SIGINT_action = {0};
        SIGINT_action.sa_handler = SIG_IGN;
        sigfillset(&SIGINT_action.sa_mask);
        SIGINT_action.sa_flags = 0;
        sigaction(SIGINT, &SIGINT_action, NULL);

        initCmd(&newCmd);
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
        if (strcmp(newCmd.cmd, "cd") == 0)
        {
            char newdir[256];
            if (strcmp(newCmd.nargv[1], ""))
            {
                //if cd is successful (path found), chdir returns 0
                if (chdir(newCmd.nargv[1]) == 0)
                {
                    fflush(stdout);
                    strcpy(newdir, getenv("PWD"));
                }

                else
                {
                    printf("Directory not found.\n");
                    fflush(stdout);
                    strcpy(newdir, getenv("PWD"));
                }
            }

            //case where no arguments provided to cd, set current directory to home environment variable value
            else 
            {
                chdir(getenv("HOME"));
                strcpy(newdir, getenv("PWD"));
            }
            setenv("PWD", newdir, 1);
            char buffer[MAXCHARS];
            getcwd(buffer, MAXCHARS);
            fflush(stdout);
        }

        else if (strcmp(newCmd.cmd, "status") == 0)
        {
            int cpstatus = 0;
            //if no foreground process has been run yet, return exit status 0
            if (fg_cpid == 0) 
            {
                printf("Exit status 0\n");
                fflush(stdout);

            }
            //
            else if (waitpid(fg_cpid, &cpstatus, WNOHANG) == 0)
            {
                printf("Exit status 0\n");
                fflush(stdout);
            }
            //the code below was adapted from the material in the "process API - monitoring child processes" section

            else if(WIFEXITED(cpstatus) != 0)
            {
            printf("Child process %d exited normally with status %d\n", cpid, WEXITSTATUS(cpstatus));
            fflush(stdout);
            } 

            else 
            {
            printf("Child process %d exited abnormally due to signal %d\n", cpid, WTERMSIG(cpstatus));
            fflush(stdout);
            }
        }

        else if (strcmp(newCmd.cmd, "exit") == 0)
        {
            //kill off all child processes and kill parent process
            sh = false;
            kill(0, SIGKILL);
        }

        else
        {
            //execute an external command
            cpid = fork();
            int cpstatus;

            switch (cpid)
            {
                case -1:
                    printf("%s failed to execute\n", newCmd.cmd);
                    fflush(stdout);
                    break;
                case 0:
                    fflush(stdout);
                    //foreground processes must terminate on receiving SIGINT
                    if (newCmd.bg == 0) {
                        SIGINT_action.sa_handler = SIG_DFL;
                        sigaction(SIGINT, &SIGINT_action, NULL);
                    }

                    //the following I/O redirection code was adapted from the lecture material for "Processes and I/O"
                    if (strcmp(newCmd.infile, "")) 
                    {
                        int inf = open(newCmd.infile, O_RDONLY);
                        if (inf == -1) 
                        {
                            fprintf(stderr, "Could not open input file '%s'\n", newCmd.infile);
                            break;
                        }
                        int openinf = dup2(inf, 0);
                        if (openinf == -1) 
                        {
                            perror("Error opening input file\n");
                            break;
                        }
                        fcntl(inf, F_SETFD, FD_CLOEXEC);
                    }

                    if (strcmp(newCmd.outfile, ""))
                    {
                        int outf = open(newCmd.outfile, O_WRONLY | O_TRUNC | O_CREAT, 0666);
                        if (outf == -1) {
                            fprintf(stderr, "Could not open ouput file '%s'\n", newCmd.outfile);
                        }
                        int openoutf = dup2(outf, 1);
                        if (openoutf == -1) {
                            perror("Error opening output file\n");
                        }
                        fcntl(outf, F_SETFD, FD_CLOEXEC);
                    }
                    char* execargv[MAXARGS];
                    for (int i = 0; i < newCmd.nargc; i++)
                    {
                        execargv[i] = strdup(newCmd.nargv[i]);
                    }
                    execvp(newCmd.cmd, execargv);
                    fprintf(stderr, "Could not execute command '%s'\n", newCmd.cmd);
                    break;

                default:            
                    //handle background commands
                    
                    
                    if (newCmd.bg != 0 && allow_bg == true)
                    {
                        
                        bg_procs[bg_procs_index] = cpid;
                        ++bg_procs_index;
                        printf("Executing command '%s' with process ID %d in background\n", newCmd.cmd, cpid);
                        fflush(stdout);
                        break;
                    }

                    //handle foreground commands
                    else
                    {
                        fg_cpid = cpid;
                        waitpid(cpid, &cpstatus, 0);
                        break;
                    }
            }
        }
        //check the status of all processes running in background before looping again. show process ID and exit status of any terminated bg commands
        int cpstatus;
        for (int i = 0; i < bg_procs_index; i++)
        {
            if (waitpid(bg_procs[i], &cpstatus, WNOHANG) != 0)
            {
                if(WIFEXITED(cpstatus) != 0)
                {
                printf("Background process %d exited normally with status %d\n", bg_procs[i], WEXITSTATUS(cpstatus));
                fflush(stdout);
                } 

                else 
                {
                printf("Background process %d exited abnormally due to signal %d\n", bg_procs[i], WTERMSIG(cpstatus));
                fflush(stdout);
                }
                --bg_procs_index;
            }
        }

        
    }
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
 * void initCmd(struct Command* cmd)
 * 
 * initialize all data members of a command struct so that it can be
 * used/reused during a given iteration of the main loop
*********************************************************************/
void initCmd(struct Command* cmd)
{
    /*
    char line[MAXCHARS];
    char cmd[ARGLEN];
    char nargv[MAXARGS][ARGLEN];
    char infile[ARGLEN];
    char outfile[ARGLEN];
    int nargc;
    int bg;
    */

    strcpy(cmd->line, "");
    strcpy(cmd->cmd, "");
    strcpy(cmd->infile, "");
    strcpy(cmd->outfile, "");
    cmd->nargc = 0;
    cmd->bg = 0;
    for (int i = 0; i < MAXARGS; ++i)
    {
        strcpy(cmd->nargv[i], "");
    }
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
    char* token;
    char* saveptr;
    char* buffer = strdup(newCmd->line);

    //first token is the command, save it as both the command and the first entry in args[]
    token = strtok_r(buffer, " ", &saveptr);
    strcpy(newCmd->nargv[0], token);
    strcpy(newCmd->cmd, token);
    ++newCmd->nargc;

    while ((token = strtok_r(NULL, " ", &saveptr))!= NULL)
    {
        //handle input file delimiter, immediately get the next token and store it as the command's infile
        if (strcmp(token, "<") == 0) 
        {
            token = strtok_r(NULL, " ", &saveptr);
            strcpy(newCmd->infile, token);
        }

        //handle the output file delimiter, immediately get the next token and store it as the command's outfile
        else if (strcmp(token, ">") == 0) 
        {
            token = strtok_r(NULL, " ", &saveptr);
            strcpy(newCmd->outfile, token);
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
                strcpy(newCmd->nargv[newCmd->nargc], token);
                ++(newCmd->nargc);
        }
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
    if (strcmp(cmd->line, ""))
    {
       printf("Line: %s\n", cmd->line); 
    }
    if (strcmp(cmd->cmd, ""))
    {
       printf("Cmd: %s\n", cmd->cmd); 
    }

    printf("Arguments: \n");
    for (int i = 0; i < cmd->nargc; i++)
    {
        printf("Arg %d: %s\n", i, cmd->nargv[i]);
    }
    if (strcmp(cmd->infile, ""))
    {
        printf("Input file is: %s\n", cmd->infile);
    }
    if (strcmp(cmd->outfile, ""))
    {
        printf("Output file is: %s\n", cmd->outfile);
    }
    if (cmd->bg == 1)
    {
        printf("Command will run in background.\n");
    }
}


void handle_SIGTSTP(int sig)
{
    char* off = "Background commands are now disabled\n";
    char* on = "Background commands are now enabled\n";
    allow_bg = !allow_bg;
    if(allow_bg)
    {
        write(STDOUT_FILENO, on, 37);
    }
    else
    {
        write(STDOUT_FILENO, off, 38);
    }
}