#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <netdb.h> 
#include <stdlib.h>

#define BUFFSIZE 1024

void setupAddressStruct(struct sockaddr_in* address, int portNumber, char* hostname);

int main(int argc, char* argv[])
{
    int client_socket;
    struct sockaddr_in server_addr;
    char hostname[] = "localhost";
    char message[BUFFSIZE];
    int message_length = strlen(message);
    char header[BUFFSIZE];
    int sent, recvd;
    char recv_buffer[BUFFSIZE];
    


    //check coomand syntax
    if(argc < 4)
    {
        perror("Use: enc_client <plaintext> <key> <port>\n");
        exit(EXIT_FAILURE);
    }

    //build the message to send to the server
    strcat(message, argv[1]);
    strcat(message, argv[2]);

    //build the header to send to the server
    sprintf(header, "%d", message_length);
    
    //build the client socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0); 
    if (client_socket < 0){
        perror("CLIENT: ERROR opening socket\n");
        exit(EXIT_FAILURE);
    }

    //build the server address struct with localhost as hostname and the server port the same as the enc_server port
    setupAddressStruct(&server_addr, atoi(argv[3]), hostname);

    // Connect to server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
    perror("CLIENT: ERROR connecting");
    exit(EXIT_FAILURE);
    }

    //send the header message to the server containing the message length
    sent = send(client_socket, header, strlen(header), 0);
    if (sent < 0)
    {
        perror("Could not send to server.\n");
        exit(EXIT_FAILURE);
    }

    //send the message to the server
    sent = send(client_socket, message, message_length, 0);
    if (sent < 0)
    {
        perror("Could not send to server.\n");
        exit(EXIT_FAILURE);
    }

    //recieve the ciphertext from the server
    memset(recv_buffer, '\0', BUFFSIZE);
    recvd = recv(client_socket, recv_buffer, BUFFSIZE, 0);
    if (recvd < 0)
    {
        perror("Did not recieve response from server.\n");
        exit(EXIT_FAILURE);
    }
    printf("%s\n", recv_buffer);


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
    fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
    exit(0); 
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr, 
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}