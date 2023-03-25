#include "server.h"
#define server_num 5
#define time_length 20

int server_ports[server_num];
int time_array[time_length];
int time_index=0;
int server_socks[5];
int client_socks[5];
int log_read = 0;
struct log_entry logs[10];
bool is_primary;
int server_sock; //socket to connect primary
pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;



int createServerSock(int send_side){
    int sockfd;
    struct sockaddr_in servaddr;


    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&servaddr, '0', sizeof(servaddr));

    /* Hardcoded IP and Port for every client*/
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port=0;
    

    /* Bind the socket to a specific port or connect to the other server abd ready to send message*/
    if(send_side){
        if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))!= 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    }
    else{
        if (bind(sockfd, (const struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        printf("\nBind failed\n");
        return -1;
    }
    }
    return sockfd;
}

int createClientSock(char* name){
    int sockfd;
    struct sockaddr_in cli_addr;
    struct hostent* host;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
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
        return -1;
    }
    return sockfd;
}

void *connection_handler(struct arg_struct *args){ //handler to deal with client request
    int client_socket = *(args->arg1);
    char option[10];
    strcpy(option,args->arg2);
    int read_size;
    char type,tittle,content;
    char client_message[200];
    memset(client_message, 0, 1024);
    while((read_size = recv(client_socket, client_message, 2000, 0)) > 0)
    {   //break down the message into different parts
        int j = sprintf_s(client_message, sizeof(client_message), "%s,%s,%s",type,tittle,content);
        printf("Client[%d]: %s", client_socket, client_message);
        if(strcmp(option,"primary_backup")==0){
            primary(type,tittle,content);
        }
        else if(strcmp(option,"quorum")==0){
            quorum(type,tittle,content);
        }
        else if(strcmp(option,"local_write")==0){
            local_write(type,tittle,content);
        }
        client_message[read_size] = '\0';
        // write(sock, message_to_client, strlen(message_to_client));
        // memset(client_message, 0, 2000);
    }
}

void post(int timestamp,char* title,char* content){
    pthread_mutex_lock(&log_lock);
    logs[log_read].timestamp=timestamp;
    logs[log_read].title = title;
    logs[log_read].content= content;
    log_read++;
    pthread_mutex_unlock(&log_lock);
}

void print_reply(int index,int loop){//recursively print every reply
    while(loop>0){
        printf("    ");
        loop--;
    }
    printf("%s\n",logs[index].title);
    if(logs[index].reply_indexes[0]){
        int i=0;
        while(logs[index].reply_indexes[i]){
            print_reply(logs[index].reply_indexes[i],loop+1);
            i++;
        }
    }
    else{
        return;
    }

}

void read_list(int number){
    pthread_mutex_lock(&log_lock);
    for(int i,j=0;i<number;i++){
        printf("%s\n",logs[i].title);
        if(logs[i].type){// for non-reply print title and its replies
            printf("%s\n",logs[i].title);
            print_reply(logs[i].reply_indexes[0],1);
        }
    }
    pthread_mutex_lock(&log_lock);
}

void choose(char* title){// match titile then print content
    pthread_mutex_lock(&log_lock);
    for(int i=0;i<10;i++){
        if (strcmp(title,logs[i].title)==0){
            printf("%s\n",logs[i].content);
            break;
        }
    }
    pthread_mutex_lock(&log_lock);
}

int next_avaiable_index(void* array[]){ //find next empty spot in an array
    for(int i=0;i<sizeof(*array)/sizeof(array[0]);i++){
        if(array[i]==0){
            return i;
        }
    }
}

void reply (int timestamp, char* content, char* title){
    pthread_mutex_lock(&log_lock);
    char reply_title[30];
    for(int i=0;i<10;i++){
        if (strcmp(title,logs[i].title)==0){    //for match title update its reply indexes and post it
            int next = next_avaiable_index(logs[i].reply_indexes);
            char str[30];
            logs[i].reply_indexes[next]=log_read;
            sprintf(str,"A reply to Article %d",logs[i].title[0]); //timestamp might get changed only the first number is persistant
            post(timestamp,"A reply to",content);
            logs[i].timestamp = timestamp;//update timestamp of  the article you reply
        }
    }
    pthread_mutex_unlock(&log_lock);
}
// void *receiveMessage(int* socket)
// {
//     char buffer[1024];
//     memset(buffer, 0, 1024);
//     while (1){
//         if (recvfrom(sockfd, buffer, 1024, 0, NULL, NULL) > 0){//primary send timestamp otherwise send logs[] or an array of timestamp?  buffer[0]then it is a timestamp else it is unmatched log entries
//             if(buffer[1]==logs[log_read].timestamp){//already synchronized
//                 is_primary=1;
//                 send()//make primary not primary
//             }
//             else{
//                 for(int i =0;i<log_read)
//                 send()//send an array of timestamp
                
//                 break;
//             }
//         }
//         memset(buffer, 0, 1024);
//     }
// }

void *receiveMessage(int* socket)
{
    char buffer[1024];
    memset(buffer, 0, 1024);
    while (1){
        if (recv(*socket, buffer, 1024, 0) > 0){//primary send timestamp otherwise send logs[] or an array of timestamp?
            if(sizeof(buffer)==sizeof(logs)){//already synchronized
                is_primary=1;
                send(*socket,"not_primary",strlen("not_primary"),0);//make primary not primary
                break;
            }
            else{
                is_primary=1;
                memcpy(&logs,&buffer,sizeof(buffer));
                break;
            }
        }
        memset(buffer, 0, 1024);
    }
}

int sendMessage(int* socket,void* message){
    int written_num= write(*socket, message, strlen(message) + 1);
    if(written_num<0){
        perror("written failed\n");
        return 0;
    }
    return 1;
    
}

void connect_primary(){//connect primary and synchronize logs
    struct sockaddr serverName;
    int server_socket;
    socklen_t size;
    while((server_socket = accept(server_sock, (struct sockaddr *)&serverName, (socklen_t *)&size))){
        printf("primary server connected");
        receiveMessage(&server_socket);
    }
}

void *broadcast_handler(struct broadcast_args* arguments){// not exiting until receive acknowledgement
    int socket = *(arguments->arg1);
    char message[1024];
    strcpy(message,arguments->arg2);
    sendMessage(&socket,&logs);
    char buffer[1024];
    memset(buffer, 0, 1024);
    if(send(socket,message,1024, 0)<0){
        perror("broadcast send fail\n");
        }
    while(1){
        if(recv(socket, buffer, 1024, 0) > 0){
            if(strcmp(&buffer[0],"ack")==0){
                break;
            }
        }
        if(send(socket,message,1024,0)>0){

        }

    }
        
}

int broadcast(){
    pthread_t broadcast_threads[server_num-1];
    int sockets[server_num-1];
    struct broadcast_args *arguments[server_num-1];
    for(int i=0;i<server_num-1;i++){
        sockets[i]=createServerSock(1);
        arguments[i]->arg1=&sockets[i];
        memcpy(&(arguments[i]->arg2),&logs,sizeof(logs));
        if( pthread_create( &broadcast_threads[i], NULL, (void *)broadcast_handler, (void*) arguments[i]) < 0){
                    perror("could not create thread");
                    return -1;
        }
    }
    for(int i=0;i<server_num-1;i++){
        if(pthread_join(broadcast_threads[i], NULL)!=0){ //all the servers receive ack then broadcast ends
            perror("broadcast not receiving ack");
            exit(0);
        }
    }
    return 1;
}

void local_write(char* type,char* title,char* content){
    int number;
        if(strcmp("Post",type)==0){
            if(is_primary){
                post(time_index++,title,content);
            }
            else{
                connect_primary(); //synchonized with the primary
                post(time_index++,title,content); //update locally
                broadcast(); //get every server synchronized
            }
        }
        else if(strcmp("Reply",type)==0){
            if(is_primary){
                reply(time_index++,title,content);
            }
            else{
                connect_primary(); //synchonized with the primary
                reply(time_index++,title,content); //update locally
                broadcast(); //get every server synchronized
            }
        }
        else if(strcmp("Choose",type)==0){
            if(is_primary){
                //wait for a heartbeat/broadcast() time that goes back and forth to a primary
                choose(title);
            }
            else{
                sleep(2);//wait for possible propagation from primary
                choose(title);
            }
        }
        else if(strcmp("Read",type)==0){
            scanf("%d",&number);
            if(is_primary){ 
            }
            else{
                sleep(2);//wait for possible propagation from primary
            }
            read_list(number);
        }
        
        

        

}

int main(int argc, char *argv[]){
    int client_sock,client_socket;
    socklen_t size;
    pthread_t client_thread;
    char option[10];
    struct arg_struct *args;
    struct sockaddr clientName;
    for(int l=0;l<10;l++){
        for(int n=0;n<20;n++){
            logs[l].reply_indexes[n]=0;
            }
    }
    
    while(1){
        client_sock = createClientSock(argv[1]);
        strncpy(option,argv[2],10);
        if(listen(client_sock,1) < 0){ //listen for the client 
            perror("Could not listen for connections\n");
            exit(0);
        }
        if(is_primary){
            for (int i=0;i<server_num;i++){ // need connect to each server to get them sychronized
            server_sock = createServerSock(1);
            server_socks[i]=server_sock;
            }
        }
        else{
            server_sock = createServerSock(0);
            if(listen(server_sock,1) < 0) {
                perror("Could not listen for connections\n");
                exit(0);
            }
            server_socks[0]=server_sock;
            }
        while(1){
            while(( client_socket = accept(client_sock, (struct sockaddr *)&clientName, (socklen_t *)&size))){
                printf("A Client connected!\n");
                args->arg1=&client_socket;
                strcpy(args->arg2,option);
                if( pthread_create( &client_thread, NULL, (void *)connection_handler, (void*) args) < 0){
                    perror("could not create thread");
                    return 1;
                }
                
            }
        }
    }
    
}


//for write op, hold the lock then if the server is primary then get a timestamp and just write and propocate to the rest
// if it is not primary then check with primary over timestamp and logindex, get synchronized,get new timestamps and make
//the old primary not primary and itself primary

//for read op, check with primary