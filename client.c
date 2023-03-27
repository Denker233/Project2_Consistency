#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#define MAX_KEY_LENGTH 256
#define MAX_VALUE_LENGTH 1024
#define MAX_MESSAGE_LENGTH 1280

void send_request(struct hostent* host, int server_port, char* message) {
    // create a socket for the client
    int status;
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Error creating socket");
        exit(1);
    }

    // set the IP address and port number for the server
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr = *((struct in_addr *)host->h_addr); // or any address
    // server_address.sin_addr.s_addr = inet_addr(server_ip);
    server_address.sin_port = htons(server_port);
    printf("%d\n",server_port);


    // if (inet_pton(AF_INET, server_address.sin_addr, &server_address.sin_addr)
    //     <= 0) {
    //     printf(
    //         "\nInvalid address/ Address not supported \n");
    //     return -1;
    // }

    printf("before send\n");
    // send a message to the server
    socklen_t server_address_size = sizeof(server_address);
    if ((status
         = connect(client_socket, (struct sockaddr*)&server_address,
                   sizeof(server_address)))
        < 0) {
        printf("\nConnection Failed \n");
    }
    if (send(client_socket, message, strlen(message),0) > 0) {
        printf("send success\n");
        // perror("Error sending message to server");
        // exit(1);
    }
    printf("after send\n");

    // receive a response from the server
    char response[MAX_MESSAGE_LENGTH];
    memset(response, 0, MAX_MESSAGE_LENGTH);
    if (recv(client_socket, response, MAX_MESSAGE_LENGTH, 0) < 0) {
        perror("Error receiving message from server");
        exit(1);
    }

    printf("Server response: %s\n", response);

    // close the client socket
    close(client_socket);
}

int main(int argc, char** argv) { // ip port key value
    // read the server IP address and port number from the command-line arguments
    struct hostent* host;
    if ((host = gethostbyname("localhost")) == NULL) { //get host name for client
      perror("client host error");
      exit(1);
    }
    int server_port = atoi(argv[1]);

    // read the key and value from the command-line arguments
    char* type = argv[2];
    char* title = argv[3];
    char* content = argv[4];

    // send a PUT request to the specified server
    char message[MAX_MESSAGE_LENGTH];
    memset(message, 0, MAX_MESSAGE_LENGTH);
    snprintf(message, MAX_MESSAGE_LENGTH-1, "%s:%s:%s", type, title,content);
    send_request(host, server_port, message);

    return 0;
}