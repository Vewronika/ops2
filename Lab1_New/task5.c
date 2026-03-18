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
   // int pipe_p[2];
    //int pipe_n[2];
    int pipe_parent[2];
}child;

void work(child* children, int index, int n, int m) {
    for (int i = 0; i < n; i++) {
        if (i == index) {
            close(children[index].pipe_parent[1]);
        }
        else {
            close(children[i].pipe_parent[1]);
            close(children[i].pipe_parent[0]);
        }
    }

    int* cards = (int*)malloc(sizeof(int) * m);

    for (int i = 0; i < m; i++) {
        char card_str[PIPE_BUF];
        read(children[index].pipe_parent[0], card_str, sizeof(card_str));
        cards[i] = atoi(card_str);
    }

    printf("%d received a hand: ", getpid());
    for (int i = 0; i < m; i++) {
        printf("%d ", cards[i]);
    }
    printf("\n");


    close(children[index].pipe_parent[0]);
    free(cards);
    return;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        ERR("input");
    }
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);

    child* children = (child*)malloc(sizeof(child) * n);
    for (int i = 0; i < n; i++) {
        pipe(children[i].pipe_parent);
    }

    srand(time(NULL));

    int deck[52];
    for (int i = 0; i < 52; i++) {
        deck[i] = 0;
    }


    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        switch (pid) {
        case 0: 

            work(children, i, n, m); 
            free(children); 
            return EXIT_SUCCESS;
        case -1: ERR("fork"); 

        }
    }
    for (int i = 0; i < n; i++) {
        close(children[i].pipe_parent[0]);
    }
    for (int i = 0; i < n; i++) {
        int* cards = (int*)malloc(sizeof(int) * m);
        for (int j = 0; j < m; j++) {
            int card;
            do {
                card = rand() % 52;
            } while (deck[card] == 1);
            deck[card] = 1;
            cards[j] = card;
        }
        for (int j = 0; j < m; j++) {
            char card_str[PIPE_BUF];
            snprintf(card_str, PIPE_BUF, "%d", cards[j]);
            write(children[i].pipe_parent[1], card_str, sizeof(card_str));

        }
        free(cards);
    }


    while (wait(NULL) > 0);
    for (int i = 0; i < n; i++) {
        close(children[i].pipe_parent[1]);
    }

    free(children);
    return EXIT_SUCCESS;
}