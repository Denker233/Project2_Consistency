#include <server.h>



int server_socks = [5]
int client_socks = [5]
struct log_entry logs = [10]
bool is_primary;
pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER; 

int createServerSock(){
    int sockfd;
    struct sockaddr_in cli_addr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return;
    }

    memset(&cli_addr, '0', sizeof(cli_addr));

    /* Hardcoded IP and Port for every client*/
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_port = 0;
    cli_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    

    /* Bind the socket to a specific port */
    if (bind(sockfd, (const struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0) {
        printf("\nBind failed\n");
        return;
    }
    return sockfd;
}

int createClientSock(char name){
    int sockfd;
    struct sockaddr_in cli_addr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return;
    }

    memset(&cli_addr, '0', sizeof(cli_addr));

    if ((host = gethostbyname(name)) == NULL) { //get host name for client
      perror("Sender: Client-gethostbyname() error lol!");
      exit(1);
      }

    /* Hardcoded IP and Port for every client*/
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_port = 0;
    cli_addr.sin_addr = *((struct in_addr *)host->h_addr); // or any address
    

    /* Bind the socket to a specific port */
    if (bind(sockfd, (const struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0) {
        printf("connect failed\n");
        return;
    }
    return sockfd;
}

void *connection_handler(void *client_socket, char option){ //handler to deal with client request
    int read_size;
    char type,tittle,content;
    while((read_size = recv(&client_socket, client_message, 2000, 0)) > 0)
    {   //break down the message into different parts
        int j = sprintf_s(client_message, sizeof(client_message), "%s,%s,%s",type,tittle,content);
        printf("Client[%d]: %s", &client_socket, client_message);
        if(option=="primary_backup"){
            primary(type,tittle,content);
        }
        else if(option=="quorum"){
            quorum(type,tittle,content);
        }
        else if(option=="local_write"){
            local_write(type,tittle,content);
        }
        client_message[read_size] = '\0';
        write(sock, message_to_client, strlen(message_to_client));
        memset(client_message, 0, 2000);
    }
}

int main(int argc, char *argv[]){
    int server_sock,client_sock,client_socket
    socklen_t size;
    pthread_t client_thread;
    char option;
    
    client_sock = createClientSock(argv[1]);
    mechanism = argv[2];
    if(listen(client_sock,1) < 0){
        perror("Could not listen for connections\n");
        exit(0);
    }
    if(is_primary){
        for (int i=0;i<5;i++){ // need connect to each server to get them sychronized
        server_sock = createServerSock();
        server_socks[i]=server_sock;
        if(listen(server_sock,1) < 0) 
            {
                perror("Could not listen for connections\n");
                exit(0);}
            }
    }
    else{
        server_sock = createServerSock();
        if(listen(sock,1) < 0) 
            {
                perror("Could not listen for connections\n");
                exit(0);
            }
        server_socks[0]=server_sock;
    }
    while(1){
            while(( client_socket = accept(client_sock, (struct sockaddr *)&clientName, (socklen_t *)&size))){
                printf("A Client connected!\n");
                if( pthread_create( &client_thread, NULL, connection_handler, (void*) &client_socket, option) < 0)
                        {
                            perror("could not create thread");
                            return 1;
                        }
            }
        }
}


//for write op, hold the lock then if the server is primary then get a timestamp and just write and propocate to the rest
// if it is not primary then check with primary over timestamp and logindex, get synchronized,get new timestamps and make
//the old primary not primary and itself primary

//for read op, check with primary