#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define ERR(source) \
    (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

#define MAX_GRAPH_NODES 32

void usage(char *progname)
{
    fprintf(stderr, "Usage: %s graph start dest\n", progname);
    exit(EXIT_FAILURE);
}

void child_work(int id, int v, int adj[MAX_GRAPH_NODES][MAX_GRAPH_NODES])
{
    printf("%d:", id);
    for (int j = 0; j < v; j++)
    {
        if (adj[id][j])
            printf(" %d", j);
    }
    printf("\n");
    fflush(stdout);
}

int main(int argc, char *argv[])
{
    if (argc != 4)
        usage(argv[0]);

    const char *path = argv[1];
    int start = atoi(argv[2]);
    int dest = atoi(argv[3]);

    FILE *f = fopen(path, "r");
    if (!f)
        ERR("fopen");

    int v;
    if (fscanf(f, "%d", &v) != 1)
        ERR("fscanf");

    if (v < 1 || v > MAX_GRAPH_NODES)
        ERR("invalid number of vertices");

    if (start < 0 || start >= v || dest < 0 || dest >= v)
        ERR("invalid start/dest");

    int adj[MAX_GRAPH_NODES][MAX_GRAPH_NODES];
    memset(adj, 0, sizeof(adj));

    int u, w;
    while (fscanf(f, "%d %d", &u, &w) == 2)
    {
        if (u < 0 || u >= v || w < 0 || w >= v)
            ERR("edge out of range");
        adj[u][w] = 1;
    }

    for (int i = 0; i < v; i++)
    {
        pid_t pid = fork();
        switch (pid)
        {
            case 0:
                print_neighbors(i, v, adj);
                fclose(f);
                _exit(EXIT_SUCCESS);
            case -1:
                ERR("fork");
            default:
                break;
        }
    }

    while (wait(NULL) > 0)
        ;

    fclose(f);
    return EXIT_SUCCESS;
}