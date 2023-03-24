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


struct log_entry {
    int timestamp;
    int type;    //1 for article,0 for reply
    char title;
    char content;
    int reply_indexes[20]={ 0 };
};

struct arg_struct {
    int*  arg1;
    char arg2[10];
};

struct broadcast_args {
    void*  arg1;
    void* arg2;
};