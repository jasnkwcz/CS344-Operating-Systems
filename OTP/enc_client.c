#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <netdb.h> 
#include <stdlib.h>

#define BUFFSIZE 1024
#define MAXSIZE 70000
#define CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZ "


void setupAddressStruct(struct sockaddr_in* address, int portNumber, char* hostname);
int validateString(char* str);
int ctoi(char c);


int main(int argc, char* argv[])
{
    int client_socket;
    struct sockaddr_in server_addr;
    char hostname[] = "localhost";
    char message[MAXSIZE * 2];
    int message_length;
    char header[BUFFSIZE];
    int sent, recvd;
    char recv_buffer[MAXSIZE];
    char plaintext[MAXSIZE];
    char key[MAXSIZE];
    FILE * plain_file;
    FILE * key_file;
    int read_file;
    int write_chars;


    //check coomand syntax
    if(argc < 4)
    {
        perror("Use: enc_client <plaintext> <key> <port>\n");
        exit(EXIT_FAILURE);
    }
    
    memset(plaintext, '\0', MAXSIZE);
    memset(key, '\0', MAXSIZE);
    memset(message, '\0', 2 * MAXSIZE);

    //build the message to send to the server
    plain_file = fopen(argv[1], "r");
    fgets(plaintext, MAXSIZE * 2, plain_file);
    key_file = fopen(argv[2], "r");
    fgets(key, MAXSIZE * 2, key_file);

    if(strlen(key) < strlen(plaintext))
    {
        perror("CLIENT: key is too short to encrypt plaintext\n");
        exit(EXIT_FAILURE);
    }

    //validate input
    if (validateString(plaintext) < 0)
    {
        perror("CLIENT: Plaintext input contains invalid characters\n");
        exit(EXIT_FAILURE);
    }

    if (validateString(key) < 0)
    {
        perror("CLIENT: Key input contains invalid characters\n");
        exit(EXIT_FAILURE);
    }

    strcat(message, plaintext);
    strcat(message, key);

    printf("message contains:\n%s", message);

    //build the header to send to the server
    message_length = strlen(message);
    sprintf(header, "%d", message_length);
    
    //build the client socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0); 
    if (client_socket < 0)
    {
        perror("CLIENT: ERROR opening socket\n");
        exit(EXIT_FAILURE);
    }

    //build the server address struct with localhost as hostname and the server port the same as the enc_server port
    setupAddressStruct(&server_addr, atoi(argv[3]), hostname);

    // Connect to server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
    perror("CLIENT: could not connect to server");
    exit(EXIT_FAILURE);
    }

    //send the header message to the server containing the message length
    sent = send(client_socket, header, strlen(header), 0);
    if (sent < 0)
    {
        perror("CLIENT: Could not send to server\n");
        exit(EXIT_FAILURE);
    }

    //get ack message from server
    memset(recv_buffer, '\0', MAXSIZE);
    recv(client_socket, recv_buffer, MAXSIZE, 0);
    char ack[] = "ok";
    if (strcmp(recv_buffer, ack) != 0)
    {
        perror("CLIENT: No acknowledgement from server\n");
        exit(EXIT_SUCCESS);
    }
    memset(recv_buffer, '\0', MAXSIZE);

    //send the message to the server
    sent = 0;
    int send_index = 0;
    while(sent < message_length)
    {
        sent += send(client_socket, &message[send_index], 1024, 0);
        send_index += sent;
    }

    if (sent < 0)
    {
        perror("CLIENT: Could not send to server\n");
        exit(EXIT_FAILURE);
    }

    //recieve the ciphertext from the server
    memset(recv_buffer, '\0', MAXSIZE);
    recvd = recv(client_socket, recv_buffer, MAXSIZE, 0);
    if (recvd < 0)
    {
        perror("CLIENT: Did not recieve response from server\n");
        exit(EXIT_FAILURE);
    }

    printf("%s\n", recv_buffer);

    //close the socket and terminate
    close(client_socket);
    exit(EXIT_SUCCESS);
}

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber, 
                        char* hostname){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname(hostname); 
  if (hostInfo == NULL) { 
    perror("CLIENT: no such host\n"); 
    exit(0); 
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr, 
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

int validateString(char* str)
{
  for(int i = 0; i < strlen(str) - 1; i++)
  {
    if (ctoi(str[i]) < 0)
    {
        return -1;
    }
    if (ctoi(str[i]) > 27)
    {
        return -1;
    }
  }
  return strlen(str);
}

int ctoi(char c)
{
  char* ptr = strchr(CHARS, c);
  int index = ptr - CHARS;
  return index;
}