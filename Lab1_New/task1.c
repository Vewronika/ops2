#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void child_two(int pipes[3][2]){
    if (close(pipes[0][0]) || close(pipes[2][0]) || close(pipes[1][1]) || close(pipes[0][1])){
         ERR("close");
    }
    char c[5];
    int status;
        if ((status = read(pipes[1][0], c, 4)) > 0){
        printf("%d, number: %s\n", getpid(), c);
        c[status] = '\0';
    }
    if (status < 0)
        ERR("read from R");
    int received = atoi(c);
 srand(getpid()); 
int random_val = (rand() % 21) - 10;
    int new = received + random_val;
    char str[4];
    snprintf(str, sizeof(str), "%d", new);
    if (write(pipes[2][1], str, 4) < 0)
        ERR("write to pipe");
    return;
}

void child_one(int pipes[3][2]){
    if (close(pipes[0][1]) || close(pipes[1][0]) || close(pipes[2][1]) || close(pipes[2][0])){
         ERR("close");
    }
    char c[5];
    int status;
    if ((status = read(pipes[0][0], c, 4)) > 0){
        printf("%d, number: %s\n", getpid(), c);
        c[status] = '\0';
    }
    if (status < 0)
        ERR("read from R");
    int received = atoi(c);
srand(getpid()); 
int random_val = (rand() % 21) - 10;
    int new = received + random_val;
    char str[4];
    snprintf(str, sizeof(str), "%d", new);
    if (write(pipes[1][1], str, 4) < 0)
        ERR("write to pipe");
    return;
}

int main(int argc, char **argv){
    
    int pipes[3][2];
    for(int i = 0; i<3; i++){
        if (pipe(pipes[i])){
            ERR("pipe");
        }    
    }

                       
    switch(fork()){
        case 0: child_one(pipes);  exit(EXIT_SUCCESS);
        case -1: ERR("fork");
    }
    switch(fork()){
        case 0: child_two(pipes);  exit(EXIT_SUCCESS);
        case -1: ERR("fork");
    }

    if (close(pipes[0][0]) || close(pipes[1][0]) || close(pipes[1][1]) || close(pipes[2][1])){
         ERR("close");
    }

    char *s = "1";
    if (write(pipes[0][1], s, 2) < 0)
        ERR("write to pipe");


    char c[5];
    int status;
    if ((status = read(pipes[2][0], c, 4)) > 0){
        printf("%d, number: %s\n", getpid(), c);
        c[status] = '\0';
    }
        
        
    if (status < 0)
        ERR("read from R");
    

    return EXIT_SUCCESS;
}
