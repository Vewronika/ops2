#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

typedef struct{
    int pipe[2];
    int pipe2[2];
}single_pipe;

void child_work(single_pipe* pipes, int m, int n, int index){
    int* cards = (int*)malloc(sizeof(int)*m);
    for(int i = 0; i<m; i++){
        cards[i]=i+1;
    }
    srand(getpid());
    for(int i = 0; i<n; i++){
        if(i!=index){
            if (close(pipes[i].pipe[0]) || close(pipes[i].pipe[1]) || close(pipes[i].pipe2[0]) || close(pipes[i].pipe2[1])){
                ERR("close");
            }
                        
        }
        else if(index == i){
            if (close(pipes[i].pipe[1]) || close(pipes[i].pipe2[0])){
                ERR("close");
            }

        }
    }
    for(int j =0; j<m; j++){
            char start;
    if (read(pipes[index].pipe[0], &start, 1) < 1){
        ERR("read");
    }
    
    int number;
    do {
        number = rand() % m;   
    } while (cards[number] == -1);

    printf("I, %d choose %d\n", index, cards[number]);
    
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", cards[number]);
  
    if (write(pipes[index].pipe2[1], buf,  sizeof(buf)) !=  sizeof(buf)){
        ERR("write");
    }
    cards[number]=-1;
    }

    
    free(cards);
    close(pipes[index].pipe[0]);
    close(pipes[index].pipe2[1]); 
    free(pipes);   
    return;
}

int main(int argc, char **argv){

    if(argc!=3){
        ERR("input");
    }

    int m = atoi(argv[1]);
    int n = atoi(argv[2]);
    single_pipe *pipes = (single_pipe*)malloc(sizeof(single_pipe)*n);
    if (!pipes) ERR("malloc");

    for(int i =0; i<n; i++){
        if(pipe(pipes[i].pipe) || pipe(pipes[i].pipe2)){
            ERR("pipe");
        }
    }

    for(int i = 0; i<n; i++){
        switch(fork()){
            case 0: child_work(pipes, m, n, i); return EXIT_SUCCESS;
            case -1: ERR("Fork:");
        }
    }

    int *results = (int *)malloc(sizeof(int)*n);
    for(int i =0; i<n; i++){
        results[i]=0;
    }
    int *array = (int *)malloc(sizeof(int)*n);


    for(int j = 0; j<m; j++){
        char start = 'a';
    for(int i =0; i<n; i++){
        if (write(pipes[i].pipe[1], &start, 1) != 1)
            ERR("write");
    }
    printf("New round\n");


    
    char buf[16];

    int res = 0;
    for(int i = 0; i<n ;i++){
        usleep(0.2);
        if (read(pipes[i].pipe2[0], buf, sizeof(buf)) < (ssize_t)sizeof(buf)){
            ERR("read");
        }   
        res=atoi(buf);
        array[i]=res;
        printf("Got number %d from player %d\n", res, i);
    }

    int count=0;
    int max = 0;
    for(int i =0; i<n;i++){
        if(array[i]>=max){
            max = array[i];
        }
    }
    for(int i=0; i<n;i++){
        if(array[i]==max){
            count++;
        }
    }
    for(int i = 0; i<n; i++){
        if(array[i]==max){
            results[i]+=n / count;
        }
    }

    printf("Winners of the round: ");
    for(int i =0; i<n; i++){
        if(array[i]==max){
            printf("number %d ", i);
        }
    }
    printf("\n");

    for(int i =0; i<n; i++){
        array[i]=0;
    }

    }

    
    for (int i = 0; i < n; ++i) {
        wait(NULL);
    }
    free(results);
    free(array);
    for (int i = 0; i < n; ++i) {
        close(pipes[i].pipe[0]);
        close(pipes[i].pipe[1]);
        close(pipes[i].pipe2[0]);
        close(pipes[i].pipe2[1]);
    }
    free(pipes);
    


    return EXIT_SUCCESS;
}