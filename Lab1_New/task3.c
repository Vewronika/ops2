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

typedef struct {
    int pipe[2];
    int *parrent;
}child;

void child_work(child* children, int n, int index) {

    for (int i = 0; i < n; i++) {
        if (i != index) {
            if (close(children[i].pipe[0])) ERR("close");
            if (close(children[i].pipe[1])) ERR("close");
        }
        else {
            if (close(children[index].pipe[1])) {
                ERR("close");
            }
        }
        
    }

    if (close(children[index].parrent[0])) {
        ERR("close");
    }

    printf("My pid: %d\n", getpid()); //work
    srand(getpid());
    int k = rand() % 7 + 3;

    char attendance[PIPE_BUF];
    if ((read(children[index].pipe[0], attendance, PIPE_BUF)) < 0) {
        ERR("read");
    }

    char response[PIPE_BUF];
    snprintf(response, PIPE_BUF, "Student %d: HERE", getpid());
    printf("Student %d: HERE\n", getpid());
    
    write(children[index].parrent[1], response, sizeof(response));
    sleep(1);
    //task
    while (1) {
        struct timespec ts;
        int ms = 100 + rand() % 401;
        ts.tv_sec = 0;
        ts.tv_nsec = (long)ms * 1000000L;
        nanosleep(&ts, NULL);

        int q = rand() % 20 + 1;
        int total = k + q;
        char task[PIPE_BUF];
        snprintf(task, PIPE_BUF, "%d %d", getpid(), total);
        write(children[index].parrent[1], task, sizeof(task));

        char m;
        if ((read(children[index].pipe[0], &m, 1)) < 1) {
            ERR("read");
        }
        if (m == 'd') {
            printf("I, %d passed\n", getpid());
            if (close(children[index].pipe[0])) {
                ERR("close");
            }
            if (close(children[index].parrent[1])) {
                ERR("close");
            }
            free(children);
            return;
        }
        else {
            continue;
        }

    }
    

    if (close(children[index].pipe[0])) {
        ERR("close");
    }
    if (close(children[index].parrent[1])) {
        ERR("close");
    }
    free(children);
    return;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        ERR("Input");
    }
    int n = atoi(argv[1]);
    if (3 > n || 20 < n) {
        ERR("input");
    }

    pid_t* pids = (pid_t*)malloc(sizeof(pid_t) * n);
    int parrent[2];
    if (pipe(parrent)) {
        ERR("pipe");
    }
        
    child* children = (child*)malloc(sizeof(child) * n);
    for (int i = 0; i < n; i++) {
        if (pipe(children[i].pipe)) {
            ERR("pipe");
        }
        children[i].parrent = parrent;
    }


    pid_t pid;
    for (int i = 0; i < n; i++) {
        pid = fork();
        switch (pid) {
            case 0: child_work(children, n, i); free(pids); return EXIT_SUCCESS;
            case -1: ERR("fork");
            default: pids[i] = pid; break;
               
        }
    }

    for (int i = 0; i < n; i++) {
        close(children[i].pipe[0]);
    }
    close(parrent[1]);



    char attendance[PIPE_BUF];
    for (int i = 0; i < n; i++) {
        int j = snprintf(attendance, PIPE_BUF, "Teacher: is %d here?", pids[i]);
        write(children[i].pipe[1], attendance, sizeof(attendance));
        memset(attendance, 0, sizeof(attendance));
    }


    char response[PIPE_BUF];

    for(int i =0; i<n; i++) {
        int j = read(parrent[0], response, sizeof(response));
        if (j == 0) {
            break;
        }
        else if (j == -1) {
            ERR("read");
        }
        else {
            printf("Teacher received: %s\n", response);
        }
    }

    int* stages = (int*)malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++) {
        stages[i] = 0;
    }

    srand(time(NULL));
    int difficulties[4];
    difficulties[0] = 3 + 1 + rand() % 20;
    difficulties[1] = 6 + 1 + rand() % 20;
    difficulties[2] = 7 + 1 + rand() % 20;
    difficulties[3] = 5 + 1 + rand() % 20;

    char task[PIPE_BUF];

    while (1) {
        int j = read(parrent[0], task, sizeof(task));
        if (j == 0) {
            break;
        }
        else if (j == -1) {
            ERR("read");
        }
        else {
            
            char* result = strchr(task, ' ');
            int amount = atoi(result + 1); // amount
            size_t len = result - task;
            char pid_str[16];
            strncpy(pid_str, task, len);
            pid_str[len] = '\0';
            pid_t pid_task = (pid_t)atoi(pid_str); //pid

            for (int l = 0; l < n; l++) {
                if (pids[l] == pid_task) {
                    int dif = difficulties[stages[l]];
                    if (amount >= dif) {
                        if (stages[l] == 3) {
                            printf("Congrats, %d passed\n", pid_task);
                            char m = 'd';
                            write(children[l].pipe[1], &m, 1);
                        }
                        else {
                            printf("%d Moves to the next stage\n", pid_task);
                            char m = 'n';
                            write(children[l].pipe[1], &m, 1);
                            stages[l]++;
                        }
                    }
                    else {
                        printf("%d Needs to fix part %d\n", pid_task, stages[l]);
                        char m = 'f';
                        write(children[l].pipe[1], &m, 1);
                    }
                    break;
                }
            }

            
        }
    }


    while (wait(NULL) > 0);

    for (int i = 0; i < n; i++) {
        close(children[i].pipe[1]);
    }
    close(parrent[0]);

    free(pids);
    free(children);
    free(stages);

    return EXIT_SUCCESS;
}