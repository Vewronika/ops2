#define _GNU_SOURCE
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define ERR(source) \
    (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

    #define BUFFER_SIZE 16;


int main(int argc, char** argv){

    int p1[2], p2[2], p3[2];
    pid_t p2, p3;
    char buffer[BUFFER_SIZE];


    srand(time(NULL));


    if(pipe(p1)) ERR("pipe");
    if(pipe(p2)) ERR("pipe");
    if(pipe(p3)) ERR("pipe");


    if((pid2 = fork()) == -1) ERR("fork");

// 0 read           1 write
    if(pid2 == 0){
        close(p1[1]);
        close(p2[0]);
        close(p3[0]);
        close(p3[1]);


        char buffer[BUFFER_SIZE];
        while(1){

            ssite_t bytes_read = read(p1[0], buffer, BUFFER_SIZE);
            if(bytes_read <= 0) break;

            buffer[bytes_read] = '\0';
            

            printf("P2 [PID %d] received %s\n", getpid(), buffer);
            
            int new_value = rend() % 100;
            sprintf(buffer, BUFFER_SIZE, "%d", new_value);
            
            write(p2[1], buffer, BUFFER_SIZE);
        }


        close(p1[0]);
        close(p2[1]);
        exit(EXIT_SUCCESS);
    }


    if((pid3 = fork()) == -1) ERR("fork");


    if (pid2 == 0) {
        close(p2[1]);
        close(p3[0]);
        close(p1[0]);
        close(p1[1]);


        char buffer[BUFFER_SIZE];
        while(1){
            
            ssize_t bytes_read = read(p2[0], buffer, BUFFER_SIZE);
            if(bytes_read <= 0) break;


            buffer[bytes_read] = '\0';
            printf("P3 (PID %d) received %s\n", getpid(), buffer);


            int new_value = rend() % 100;
            sprintf(buffer, BUFFER_SIZE, "%d", new_value);
            
            write(p3[1], buffer, BUFFER_SIZE);
        }


        close(p2[0]);
        close(p3[1]);
        exit(EXIT_SUCCESS);
    }




    close(p1[0]);
    close(p2[1]);
    close(p3[1]);

    int initial_num = rand() % 100;
    sprintf(buffer, BUFFER_SIZE, "%d", initial_num);
    write(p1[1], buffer. BUFFER_SIZE);


    while(1){
        ssize_t bytes_read = read(p3[0], buffer, BUFFER_SIZE);
        if(bytes_read <= 0) break;

        buffer[bytes_read] = '\0';
        printf("P1 (parent %d) received %d\n", getpid(), buffer);


        int new_value = rend() % 100;
        sprintf(buffer, BUFFER_SIZE, "%d", new_value);
            
        write(p3[1], buffer, BUFFER_SIZE);
    }

    close(p3[0]);
    close(p1[1]);
    

    wait(NULL);
    wait(NULL);

    printf("all terminated\n");


    return EXIT_SUCCESS;
}