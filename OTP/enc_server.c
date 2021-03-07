#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>


#define BUFFSIZE 1024
#define CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZ "


int ctoi(char c);
char itoc(int i);
void setupAddressStruct(struct sockaddr_in* address, int portNumber);
int validateString(char* str);
int encryptString(char* plaintext, char* key, char* ciphertext);


int main(int argc, char* argv[])
{
  struct sockaddr_in server_addr, client_addr;
  int server_socket, client_socket, conn_socket;
  socklen_t client_size = sizeof(client_addr);
  int plaintext_size;
  int key_size;
  int cipher_size;
  int msg_size;
  int pid;
  pid_t conn_array[5];
  int conn_index = 0;
  char recv_buff[BUFFSIZE];
  ssize_t recv_size;
  int cpid_status;
  char plaintext[BUFFSIZE];
  char key[BUFFSIZE];
  char msg_buff[BUFFSIZE];
  char ciphertext[BUFFSIZE];
  char* saveptr;
  ssize_t send_size;

  //check for valid command, error and exit if no port is spec'd
  if (argc < 2)
  {
    perror("Error: enc_server expects at least 2 arguments.\n");
    exit(EXIT_FAILURE);
  }

  //create a listening socket
  setupAddressStruct(&server_addr, atoi(argv[1]));
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket < 0)
  {
    perror("Error creating server listening socket.\n");
    exit(1);
  }

  // Associate the socket to the port
  if (bind(server_socket, 
          (struct sockaddr *)&server_addr, 
          sizeof(server_addr)) < 0){
    perror("Could not bind listening socket to the server.");
  }

  //allow up to 5 connections to queue
  listen(server_socket, 5);

  while(1)
  {
    //check for any terminated child processes
    for(int i = 0; i < conn_index; i++)
    {
      if(waitpid(conn_array[i], &cpid_status, WNOHANG) == conn_array[i])
      {
        --conn_index;
      }
    }
    //wait for a connection request
    conn_socket = accept(server_socket, (struct sockaddr*)&client_addr, (socklen_t*)sizeof(client_addr));
    if (client_socket < 0)
    {
      perror("Error: could not accept connection to client socket.\n");
      continue;
    }

    //when a connection is recieved, create a new process with fork()
    pid = fork();
    
    switch(pid)
    {
      case -1:
        perror("Error in creating child process to serve client.\n");
        break;
  
      case 0:
        memset(recv_buff, '\0', BUFFSIZE);
        memset(plaintext, '\0', BUFFSIZE);
        memset(key, '\0', BUFFSIZE);
        memset(msg_buff, '\0', BUFFSIZE);
        recv_size = 0;
        char * token;

        //recv first msg from client, header specifying message length
        recv(client_socket, recv_buff, BUFFSIZE, 0);
        msg_size = atoi(recv_buff);

        //recieve into buffer until message end
        while (recv_size <= msg_size)
        {
          recv_size += recv(client_socket, recv_buff, BUFFSIZE, 0);
          strcat(msg_buff, recv_buff);
          memset(recv_buff, '\0', BUFFSIZE);
        }

        //split the strings using the newline character, then copy
        token = strtok_r(msg_buff, "\n", &saveptr);
        strcpy(plaintext, token);
        token = strtok_r(NULL, "\n", &saveptr);
        strcpy(key, token);

        //validate plaintext and key
        plaintext_size = validateString(plaintext);
        key_size = validateString(key);
        if (plaintext_size != key_size)
        {
          perror("Plaintext and Key must be the same size\n");
        }

        //encrypt the plaintext using the key using ctoi and itoc functions
        send_size = encryptString(plaintext, key, ciphertext);

        //send the encrypted string back to the client
        send(client_socket, ciphertext, send_size, 0);

        //close the client connection
        close(client_socket);
        break;

      default:
        conn_array[conn_index] = pid;
        ++conn_index;
    }
  }

  return 0;
}


//convert a character c into its corresponding integer index value in CHARS
int ctoi(char c)
{
  char* ptr = strchr(CHARS, c);
  int index = ptr - CHARS;
  return index;
}


//convert an integer into its corresponding character value in CHARS
char itoc(int i)
{
  return(CHARS[i]);
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, int portNumber)
{
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}

int validateString(char* str)
{
  for(int i = 0; i < strlen(str); i++)
  {
    if (((int)str[i] != 32) && (((int)str[i] - 65) > 25))
    {
      perror("Recieved invalid plaintext input from client.\n");
      return 0;
    }
  }
  return strlen(str);
}

int encryptString(char* plaintext, char* key, char* ciphertext)
{
  int size = strlen(plaintext);
  memset(ciphertext, '\0', BUFFSIZE);
  char cipher_char;
  int cipher_int;
  char plain_char;
  int plain_int;
  char key_char;
  int key_int;
  int length = 0;


  for (int i = 0; i < size; i++)
  {
    plain_char = plaintext[i];
    plain_int = ctoi(plain_char);

    key_char = key[i];
    key_int = ctoi(key_char);

    cipher_int = (key_int + plain_int) % 27;
    cipher_char = itoc(cipher_int);

    ++length;
  }

  return strlen(ciphertext);
}