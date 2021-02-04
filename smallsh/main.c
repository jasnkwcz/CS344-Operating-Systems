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
#define MAXPROCS 128
#define VAREXP "$$"

pid_t procs[MAXPROCS];
int cpid_index;
pid_t fg_cpid;

struct Command
{
    char *cmd;
    char **args;
    char *inFile;
    char *outFile;
    int bg;
};

void findAndReplace(char **str, char *search, char *replace);
int getUserInput(char **line);
struct Command *parseInput(char **line);
void runCmd(struct Command *cmd);
char* cd_builtin(struct Command *cmd);
void exit_builtin();
int status_builtin(pid_t cpid);
void externalCmd(struct Command *cmd);
void displayCmd(struct Command *cmd);
void clearCmd(struct Command *cmd);

int main(void)
{
    //assign custom signal handlers for SIGINT and SIGSTP
    //this code was largely adapted/inspired by the lectures for signal handling
    struct sigaction ignore_sig = {0};
    ignore_sig.sa_handler = SIG_IGN;
    sigaction(SIGINT, &ignore_sig, NULL);
    sigaction(SIGTSTP, &ignore_sig, NULL);

    //initialize varaibles
    char *nline; //a command line
    char **nargv; //array of char pointers to hold the argument vector, whose first entry is the command itself
    chdir(getenv("HOME"));
    cpid_index = 0;
    int cpstatus;

    //main command prompt loop
    while (true)
    {
        //get user input, store it in nline. if it's blank, ignore it and restart the prompt loop
        if (getUserInput(&nline) == 1) {
            continue;
        };
        if (*nline == '#') {
            continue;
        }

        //parse input into a new command
        struct Command *cmd = parseInput(&nline);

        //run the user command
        runCmd(cmd);
        //clean up and destroy the command struct
        clearCmd(cmd);
        for (int i = 0; i < cpid_index; i++)
        {
            waitpid(procs[i], &cpstatus, WNOHANG);
        }
    }
    //clean up
    return 0;
}


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
    return strlen(*line);
}

/*
parseInput()
    if the user input is blank or begins with "#", then input is ignored and returns 0
    else parse the input by splitting into tokens, storing in 
*/
struct Command* parseInput(char** line) {
    struct Command *newCmd = (struct Command *)malloc(sizeof(struct Command));
    newCmd->bg = 0;
    newCmd->args = (char **)calloc(MAXARGS, sizeof(char *));
    newCmd->inFile = NULL;
    newCmd->outFile = NULL;

    //initialize variables for strtok_r and a counter for the argument list
    char *token;
    char *saveptr;
    int nargc = 1;
    char *varex;

    //first token is the command
    token = strtok_r(*line, " \n", &saveptr);

    //initialize a variable to detect variables held in VAREXP
    varex = strstr(token, VAREXP);

    //get the current process ID and convert it to a string
        pid_t p = getpid();
        char pstr[8];
        sprintf(pstr, "%d", p);
        
    if (varex != NULL)
    {
        
        //allocate a string that is a copy of the token, so that we don't change the contents of the command line
        char *copystr = (char *)calloc(MAXCHARS, sizeof(char));
        strcpy(copystr, token);
        //replace all occurances of "$$" with the current process id
        findAndReplace(&copystr, VAREXP, pstr);
        //copy the replaced string into the command and args[0] members of the new command
        newCmd->cmd = (char *)calloc(strlen(copystr) + 1, sizeof(char));
        newCmd->args[0] = (char *)calloc(strlen(copystr) + 1, sizeof(char));
        strcpy(newCmd->cmd, copystr);
        strcpy(newCmd->args[0], copystr);
        //let go of the copy string once we are done with it and its contents have been stored into the command
        free(copystr);
    }
    else {
        newCmd->cmd = (char *)calloc(strlen(token) + 1, sizeof(char));
        newCmd->args[0] = (char *)calloc(strlen(token) + 1, sizeof(char));
        strcpy(newCmd->cmd, token);
        strcpy(newCmd->args[0], token);
    }

    //begin parsing the arguments for the command
    while ((token = strtok_r(NULL, " \n", &saveptr))!= NULL)
    {
        //handle input file delimiter, immediately get the next token and store it as the command's infile
        if (strcmp(token, "<") == 0) {
            token = strtok_r(NULL, " \n", &saveptr);
            newCmd->inFile = (char *)calloc(strlen(token) + 1, sizeof(char));
            strcpy(newCmd->inFile, token);
        }

        //handle the output file delimiter, immediately get the next token and store it as the command's outfile
        else if (strcmp(token, ">") == 0) {
            token = strtok_r(NULL, " \n", &saveptr);
            newCmd->outFile = (char *)calloc(strlen(token) + 1, sizeof(char));
            strcpy(newCmd->outFile, token);
        }

        //handle commands running in the background
        else if (strcmp(token, "&") == 0) {
            if ((token = strtok_r(NULL, " \n", &saveptr)) == NULL)
            {
                newCmd->bg = 1;
            }
        }

        //handle everything else (command arguments)
        else {
            newCmd->args[nargc] = (char *)calloc(strlen(token) + 1, sizeof(char));
            findAndReplace(&newCmd->args[nargc], VAREXP, pstr);
            strcpy(newCmd->args[nargc], token);
            ++nargc;
        }
        newCmd->args[nargc] = NULL;
    }

    return newCmd;
}

/*
void findAndReplace()
    handles variable expansion when a "$$" is found in a command
    takes the command string as input, replaces the "$$" with the current process ID
    replace command string with the new string
    this code was largely inspired and adapted from the post found
    here: https://stackoverflow.com/questions/32413667/replace-all-occurrences-of-a-substring-in-a-string-in-c/32413923
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
        printf("Current directory: %s\n", buffer);
        fflush(stdout);

    }

    else if (strcmp(cmd->cmd, "status") == 0)
    {
        status_builtin(fg_cpid);
    }

    else if (strcmp(cmd->cmd, "exit") == 0)
    {
        exit_builtin();
    }

    else
    {
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
    
    int cpstatus;
    switch (cpid)
    {
        case -1:
            printf("%s failed to execute\n", cmd->cmd);
            fflush(stdout);
            exit(1);
            break;
        case 0:
            fflush(stdout);
            //the following I/O redirection code was adapted from the lecture material for "Processes and I/O"
            if (cmd->inFile != NULL) 
            {
                printf("infile is: %s\n", cmd->inFile);
                int inf = open(cmd->inFile, O_RDONLY);
                if (inf == -1) {
                    perror("Could not open file");
                    exit(1);
                }
                int openinf = dup2(inf, 0);
                if (openinf == -1) {
                    perror("error opening file in child process");
                    exit(1);
                }
            }

            if (cmd->outFile != NULL)
                
            {
                printf("outfile is: %s\n", cmd->outFile);
                int outf = open(cmd->outFile, O_WRONLY | O_TRUNC | O_CREAT);
                if (outf == -1) {
                    perror("Could not open file");
                    exit(1);
                }
                int openoutf = dup2(outf, 0);
                if (openoutf == -1) {
                    perror("error opening file in child process");
                    exit(1);
                }
            }
            execvp(cmd->cmd, cmd->args);
            perror("execvp failed");
            exit(EXIT_FAILURE);

        default:            
            //handle background commands
            
            procs[cpid_index] = cpid;
            ++cpid_index;
            if (cmd->bg != 0)
            {
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

/*
Built-in command: cd_builtin()
    runs in foreground only
    if no args are specified, changes to the directory pointed to by the HOME environment variable
    takes up to one arg: a relative OR absolute path to a directory to change to
*/
char* cd_builtin(struct Command *cmd)
{
    //handle the case where there is a directory path specified, change to specified directory
    if (cmd->args[1] != NULL) {
        //if cd is successful (path found), chdir returns 0
        if (chdir(cmd->args[1]) == 0)
        {
            printf("%s\n", cmd->args[1]);
            fflush(stdout);
            return (getenv("PWD"));
        }

        else
        {
            printf("Directory not found.\n");
            fflush(stdout);
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
int status_builtin(pid_t cpid)
{
    //sanity check, if no child process has executed yet
    int cpstatus = 0;

    if (waitpid(cpid, &cpstatus, WNOHANG) == 0) {
        return (EXIT_FAILURE);
    };
    //the code below was adapted from the material in the "process API - monitoring child processes" section

    if(WIFEXITED(cpstatus)){
      printf("Child process { %d } exited normally with status %d\n", cpid, WEXITSTATUS(cpstatus));
      fflush(stdout);
    } else{
      printf("Child process { %d } exited abnormally due to recieving signal %d\n", cpid, WTERMSIG(cpstatus));
      fflush(stdout);
    }
    return (EXIT_SUCCESS);
}

/*
void clearCmd(struct Command *cmd)
    cleans up all memory from a struct
*/
void clearCmd(struct Command *cmd)
{
    free(cmd->cmd);
    if (cmd->args != NULL) 
    {
        for (int i = 0; i < MAXARGS; ++i)
        {
            free(cmd->args[i]);
        }
        free(cmd->args);
    }

    if (cmd->inFile != NULL) {
        free(cmd->inFile);
    }
    if (cmd->outFile != NULL) {
        free(cmd->outFile);
    }
    free(cmd);
}

/*
void displaytCmd(struct Command * cmd)
    this function is used for debugging purposes only
*/
 void displayCmd(struct Command *cmd)
 {
     printf("Current command: \n");
     fflush(stdout);
     int count = 0;
     printf("cmd: %s\n", cmd->cmd);
     fflush(stdout);
     printf("args: \n");
     fflush(stdout);
     for (int i = 0; cmd->args[i] != NULL; ++i)
     {
         ++count;
         printf("%s\n", cmd->args[i]);
         fflush(stdout);
     }
     printf("%s has %d arguments\n", cmd->cmd, count);
     fflush(stdout);
     if (cmd->inFile != NULL)
     {
         printf("infile: %s\n", cmd->inFile);
         fflush(stdout);
     }
     if (cmd->outFile != NULL) 
     {
         printf("outfile: %s\n", cmd->outFile);
         fflush(stdout);
     }
     printf("background? : %d\n", cmd->bg);
     fflush(stdout);
 }
 