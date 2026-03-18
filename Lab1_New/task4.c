
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

int sethandler(void (*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;
    if (-1 == sigaction(sigNo, &act, NULL))
        return -1;
    return 0;
}

typedef struct{
    int pipe[2];
    int *parent;

}child;

void child_work(int* money, int index, int n, child* children){
    for(int i = 0; i<n; i++){
        if(i!=index){
            close(children[i].pipe[1]);
            close(children[i].pipe[0]);
        }else{
            close(children[i].pipe[0]);
        }
    }
    close(children[index].parent[1]);

    printf("I, %d have an amount %d and I will play roulette\n", getpid(), money[index]);
    srand(getpid());

    while(1){
    int bet = rand()%36;
    int amount = rand()%money[index]+1;
    char message[PIPE_BUF];
    snprintf(message, PIPE_BUF, "%d %d", amount, bet);

    write(children[index].pipe[1], message, sizeof(message));

    char winner[PIPE_BUF];
    read(children[index].parent[0], winner, sizeof(winner));
    if(atoi(winner)==bet){
        printf("I %d won %d\n", getpid(), amount * 35);
        money[index]+=amount*35;
    }
    else{
        printf("I %d lost %d\n", getpid(), amount);
        money[index]-=amount;
    }
    if(money[index]<=0){
        printf("I %d am broke\n", getpid());
        break;
    }
    }

    
    close(children[index].pipe[1]);
    close(children[index].parent[0]);

    return;
}

int main(int argc, char **argv){
    
    if(argc!=3){
        ERR("input");
    }
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);

    int (*parent)[2] = malloc(n * sizeof(int[2]));
    for (int i = 0; i < n; ++i) pipe(parent[i]);
    

    child *children = (child*)malloc(sizeof(child)*n);
    for(int i = 0; i<n; i++){
        if(pipe(children[i].pipe)){
            ERR("pipe");
        }
        children[i].parent = parent[i];
    }
    int *money = (int*)malloc(sizeof(int)*n);
    for(int i =0; i<n; i++){
        money[i]=m;
    }
    pid_t* pids = (pid_t*)malloc(sizeof(pid_t)*n);
    
    
    for(int i =0; i<n; i++){
        pid_t pid = fork();
        switch(pid){
            case 0: child_work(money, i, n, children); free(money); free(children); free(pids); free(parent); return EXIT_SUCCESS;
            case -1: ERR("fork");
            default: pids[i] = pid; break;
        }
    }

    for(int i =0; i<n; i++){
        close(children[i].pipe[1]);
    }
    for(int i=0; i<n;i++){
        close(parent[i][0]);
    }
    
    srand(time(NULL));


    while(1){
        int winner = rand()%35;


    int betters = 0;
    for(int i =0; i<n; i++){
        
        char message[PIPE_BUF];
        int j = read(children[i].pipe[0], message, sizeof(message));
        if(j==0){
            continue;
        }
        betters++;
        char* result = strchr(message, ' ');
        int bet = atoi(result + 1);
        size_t len = result - message;
        char amount_str[16];
        strncpy(amount_str, message, len);
        amount_str[len] = '\0';
        int amount = atoi(amount_str); 
        printf("Dealer: %d placed %d on %d\n", pids[i], amount, bet);
        if(bet == winner){
            money[i]+=amount*35;
        }else{
            money[i]-=amount;
        }
    }
    if(betters==0){
        break;
    }
   
    printf("Dealer: %d is the lucky number\n", winner);
    char winner_str[PIPE_BUF];
    snprintf(winner_str, PIPE_BUF, "%d", winner);
    for(int j =0; j<n; j++){
  
        write(parent[j][1], winner_str, sizeof(winner_str));
    }

    
    }
    printf("Dealer: Casino always wins\n");
    
    
    for(int i =0; i<n; i++){
        close(children[i].pipe[0]);
        close(children[i].pipe[1]);
    }
    for(int i=0; i<n;i++){
        close(parent[i][1]);
    }
    while(wait(NULL)>0){

    }

    free(children);
    free(money);
    free(pids);
    free(parent);
    return EXIT_SUCCESS;
}