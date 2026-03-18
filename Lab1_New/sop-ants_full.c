

int main(int argc, char* argv[])
{
    set_handler(SIG_IGN, SIGINT);
    set_handler(SIG_IGN, SIGPIPE);
    unlink(FIFO_NAME);
    mkfifo(FIFO_NAME, 0666);
    if (argc != 4)
        usage(argc, argv);
    int starting_index = atoi(argv[2]);
    int destination_index = atoi(argv[3]);
    graph_t graph = read_colony(argv[1]);
    for(int i = 0; i < graph.node_num; i++){
        if(pipe(graph.nodes[i].pipe) == -1){
            ERR("pipe");
        }
    }
    for(int i = 0; i < graph.node_num; i++){
        pid_t pid = fork();
        if(pid == -1){
            ERR("fork");
        }
        else if(pid == 0){
            int fd_w[MAX_GRAPH_NODES];
            for(int j = 0; j < graph.node_num; j++){
                if(i == j){
                    close(graph.nodes[i].pipe[1]);
                }
                else{
                    close(graph.nodes[j].pipe[0]);
                    fd_w[j] = graph.nodes[j].pipe[1];
                }
            }
            child_work(graph.nodes[i], i, fd_w, destination_index);
            for(int j = 0; j < graph.node_num; j++){
                if(i == j){
                    close(graph.nodes[i].pipe[0]);
                }
                else{
                    close(graph.nodes[j].pipe[1]);
                }
            }
            exit(EXIT_SUCCESS);
        }
    }    
    for(int i = 0; i < graph.node_num; i++){
        if(i == starting_index){
            close(graph.nodes[i].pipe[0]);
        }
        else{
            close(graph.nodes[i].pipe[0]);
            close(graph.nodes[i].pipe[1]);
        }
    }    

    int fd = open(FIFO_NAME, O_RDONLY | O_NONBLOCK);
    if(fd == -1){
        ERR("open");
    }


    int ID = 0;
    while(1){
        msleep(1000);
        ant_t ant = {};
        ant.ID = ID++;
        if(write(graph.nodes[starting_index].pipe[1], &ant, sizeof(ant)) == -1){
            if(errno == EPIPE){
                break;
            }
            ERR("write");
        }
        if(read(fd, &ant, sizeof(ant)) < 0){
            if(errno == EAGAIN){
                continue;
            }
            ERR("read");
        }
        printf("Ant {%d} path: ", ant.ID);
        for(int i = 0; i < ant.path_length; i++){
            printf("%d ", ant.path[i]);
        }
        printf("\n");
    }

    
    kill(0, SIGINT);
    while(wait(NULL) > 0){}
    close(graph.nodes[starting_index].pipe[1]);
    close(fd);
    unlink(FIFO_NAME);
    exit(EXIT_SUCCESS);
}