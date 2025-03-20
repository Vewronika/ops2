#define _GNU_SOURCE
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define SIZE 16
#define ERR(source) \
    (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

void modify_and_send(int read_fd, int write_fd, const char *proc_name) {
    char buffer[SIZE];
    while (1) {
        ssize_t bytes_read = read(read_fd, buffer, SIZE);
        if (bytes_read <= 0) break;

        buffer[bytes_read] = '\0';
        int num = atoi(buffer);
        printf("%s (PID %d) received %d\n", proc_name, getpid(), num);

        if (num == 0) break;

        int random_change = (rand() % 21) - 10;
        num += random_change;

        snprintf(buffer, SIZE, "%d", num);
        if (write(write_fd, buffer, SIZE) == -1) {
            perror("write failed");
            break;
        }
    }
}

int main() {
    int p1[2], p2[2], p3[2];
    pid_t pid2, pid3;

    srand(time(NULL));
    if (pipe(p1) || pipe(p2) || pipe(p3)) ERR("pipe");

    if ((pid2 = fork()) == -1) ERR("fork");
    if (pid2 == 0) {
        close(p1[1]); close(p2[0]); close(p3[0]); close(p3[1]);
        modify_and_send(p1[0], p2[1], "P2");
        close(p1[0]); close(p2[1]);
        exit(EXIT_SUCCESS);
    }

    if ((pid3 = fork()) == -1) ERR("fork");
    if (pid3 == 0) {
        close(p2[1]); close(p3[0]); close(p1[0]); close(p1[1]);
        modify_and_send(p2[0], p3[1], "P3");
        close(p2[0]); close(p3[1]);
        exit(EXIT_SUCCESS);
    }

    close(p1[0]); close(p2[1]); close(p3[1]);

    int initial_num = 1;
    char buffer[SIZE];
    snprintf(buffer, SIZE, "%d", initial_num);
    if (write(p1[1], buffer, SIZE) == -1) ERR("write");

    modify_and_send(p3[0], p1[1], "P1 (Parent)");
    
    close(p3[0]); close(p1[1]);
    wait(NULL); wait(NULL);
    printf("All processes terminated.\n");
    return EXIT_SUCCESS;
}
