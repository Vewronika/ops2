#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define MAX_KNIGHT_NAME_LENGHT 20

#define ERR(source) \
    (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

int set_handler(void (*f)(int), int sig)
{
    struct sigaction act = {0};
    act.sa_handler = f;
    if (sigaction(sig, &act, NULL) == -1)
        return -1;
    return 0;
}

void msleep(int millisec)
{
    struct timespec tt;
    tt.tv_sec = millisec / 1000;
    tt.tv_nsec = (millisec % 1000) * 1000000;
    while (nanosleep(&tt, &tt) == -1)
    {
    }
}
    
typedef struct{
  char *params;
  char name[64];
  int hp;
  int attack;
  int pipe[2];
}knight;
 
int count_descriptors()
{
    int count = 0;
    DIR* dir;
    struct dirent* entry;
    struct stat stats;
    if ((dir = opendir("/proc/self/fd")) == NULL)
        ERR("opendir");
    char path[PATH_MAX];
    getcwd(path, PATH_MAX);
    chdir("/proc/self/fd");
    do
    {
        errno = 0;
        if ((entry = readdir(dir)) != NULL)
        {
            if (lstat(entry->d_name, &stats))
                ERR("lstat");
            if (!S_ISDIR(stats.st_mode))
                count++;
        }
    } while (entry != NULL);
    if (chdir(path))
        ERR("chdir");
    if (closedir(dir))
        ERR("closedir");
    return count - 1;  // one descriptor for open directory
}


void frank_work(int f_n, int s_n, knight* f_k, knight* s_k, int index){
  for(int i = 0; i<f_n; i++){
    if(i!=index){
      if (close(f_k[i].pipe[0]) || close(f_k[i].pipe[1])){
        ERR("close");
      }
        
    }
    else{
      if(close(f_k[i].pipe[1])){
        ERR("close");
      }
    }
  }
  for(int i = 0; i<s_n; i++){
    if(close(s_k[i].pipe[0])){
      ERR("close");
    }
  }
  srand(getpid());
  printf("I am %s, hp: %d, attack: %d\n", f_k[index].name, f_k[index].hp, f_k[index].attack);
  int read_fd = f_k[index].pipe[0];
  int flags = fcntl(read_fd, F_GETFL);
  fcntl(read_fd, F_SETFL, flags | O_NONBLOCK);
  char buf[2];
  int res = 0;
  ssize_t n;
  while(1){
     while(1){
     n = read(f_k[index].pipe[0], buf, 1);
  if (n == -1) {
    if (errno == EAGAIN) {
        break;
    } else {
        ERR("read");
    }
  }
  else if (n == 0) {
    break;
  }
  else {
    buf[1] = '\0';
    res=atoi(buf);
    f_k[index].hp-=res;
  }
  }
  
  int enemy = rand()%s_n;
  char damage = rand()%f_k[index].attack;

  int write_fd = s_k[enemy].pipe[1];

  int flags_w = fcntl(write_fd, F_GETFL); 
  fcntl(write_fd, F_SETFL, flags_w | O_NONBLOCK);

  write(write_fd, &damage, 1);
  if((int)damage==0){
    printf("Frank index %d: weak attack\n", index);

  }
  else if((int)damage>=10){
    printf("Frank index %d: strong attack %d\n", index, (int)damage);
  }
  else{
    printf("Frank index %d: normal attack %d\n", index, (int)damage);
  }
    
  sleep(1);
  }

 
  for(int i = 0; i<s_n; i++){
    if(close(s_k[i].pipe[1])){
      ERR("close");
    }
  }
  if(close(f_k[index].pipe[0])){
    ERR("close");
  }
  for(int i =0; i<f_n; i++){
    free(f_k[i].params);
  }
  for(int i =0; i<s_n; i++){
    free(s_k[i].params);
  }
  free(f_k);
  free(s_k);
  return;
}


void sarac_work(int f_n, int s_n, knight* f_k, knight* s_k, int index){
  for(int i = 0; i<s_n; i++){
    if(i!=index){
      if (close(s_k[i].pipe[0]) || close(s_k[i].pipe[1])){
        ERR("close");
      }
        
    }
    else{
      if(close(s_k[i].pipe[1])){
        ERR("close");
      }
    }
  }
  for(int i = 0; i<f_n; i++){
    if(close(f_k[i].pipe[0])){
      ERR("close");
    }
  }
  srand(getpid());
  printf("I am %s, hp: %d, attack: %d\n", s_k[index].name, s_k[index].hp, s_k[index].attack);
    int read_fd = s_k[index].pipe[0];
  int flags = fcntl(read_fd, F_GETFL);
  fcntl(read_fd, F_SETFL, flags | O_NONBLOCK);
  char buf[2];
  int res = 0;
  ssize_t n;
  while(1){
     while(1){
     n = read(s_k[index].pipe[0], buf, 1);
  if (n == -1) {
    if (errno == EAGAIN) {
        break;
    } else {
        ERR("read");
    }
  }
  else if (n == 0) {
    break;
  }
  else {
    buf[1] = '\0';
    res=atoi(buf);
    s_k[index].hp-=res;
  }
  }
  
  int enemy = rand()%f_n;
  char damage = rand()%s_k[index].attack;

  int write_fd = f_k[enemy].pipe[1];

  int flags_w = fcntl(write_fd, F_GETFL); 
  fcntl(write_fd, F_SETFL, flags_w | O_NONBLOCK);

  write(write_fd, &damage, 1);
  if((int)damage==0){
    printf("Saracin index %d: weak attack\n", index);

  }
  else if((int)damage>=10){
    printf("Saracin index %d: strong attack %d\n", index, (int)damage);
  }
  else{
    printf("Saracin index %d: normal attack %d\n", index, (int)damage);
  }
    
  sleep(1);
  }

  for(int i = 0; i<f_n; i++){
    if(close(f_k[i].pipe[1])){
      ERR("close");
    }
  }
  if(close(s_k[index].pipe[0])){
    ERR("close");
  }
  for(int i =0; i<s_n; i++){
    free(s_k[i].params);
  }
  for(int i =0; i<f_n; i++){
    free(f_k[i].params);
  }
  free(f_k);
  free(s_k);
  return;
}

int main(int argc, char* argv[])
{
  FILE* franc;
  FILE* sarac;
  if ((franc = fopen("franci.txt", "r")) ==NULL){
    ERR("Franks have not arrived on the battlefield");
  }
  if((sarac = fopen("saraceni.txt", "r")) ==NULL){
    ERR("Saracens have not arrived on the battlefield");
  }
  int f_n;
  
  int s_n;
  if (fscanf(franc, "%d", &f_n) != 1) ERR("fscanf f_n");
  if (fscanf(sarac, "%d", &s_n) != 1) ERR("fscanf s_n");
  
  fgetc(franc); 
  fgetc(sarac);
  
  knight* f_k = calloc(f_n, sizeof(knight));
  knight* s_k = calloc(s_n, sizeof(knight));
  


  for(int i = 0; i<f_n; i++){
    size_t len = 0; 
    f_k[i].params = NULL;
    getline(&f_k[i].params, &len, franc);

    char * myPtr = strtok(f_k[i].params, " ");
    if(myPtr != NULL) {
      strcpy(f_k[i].name, myPtr);
      myPtr = strtok(NULL, " ");
    }
    if(myPtr != NULL) {
      f_k[i].hp = atoi(myPtr);
      myPtr = strtok(NULL, " ");
    }
    if(myPtr != NULL) {
      f_k[i].attack = atoi(myPtr);
      myPtr = strtok(NULL, " ");
    }
    
    //printf("I am %s, hp: %d, attack: %d\n", f_k[i].name, f_k[i].hp, f_k[i].attack);
  }
  for(int i = 0; i<s_n; i++){
    size_t len = 0; 
    s_k[i].params = NULL;
    getline(&s_k[i].params, &len, sarac);
    
    char * myPtr = strtok(s_k[i].params, " ");
    if(myPtr != NULL) {
      strcpy(s_k[i].name, myPtr);
      myPtr = strtok(NULL, " ");
    }
    if(myPtr != NULL) {
      s_k[i].hp = atoi(myPtr);
      myPtr = strtok(NULL, " ");
    }
    if(myPtr != NULL) {
      s_k[i].attack = atoi(myPtr);
      myPtr = strtok(NULL, " ");
    }
    
    //printf("I am %s, hp: %d, attack: %d\n", s_k[i].name, s_k[i].hp, s_k[i].attack);
  }
  for(int i =0; i<f_n; i++){
    pipe(f_k[i].pipe);
  }
  for(int i =0; i<s_n; i++){
    pipe(s_k[i].pipe);
  }
  
  for(int i =0; i<f_n; i++){
    
    switch(fork()){
      case 0: frank_work(f_n, s_n, f_k, s_k, i); return EXIT_SUCCESS;
      case -1: ERR("fork");
    }
  }

   for(int i =0; i<s_n; i++){
    
    switch(fork()){
      case 0: sarac_work(f_n, s_n, f_k, s_k, i); return EXIT_SUCCESS;
      case -1: ERR("fork");
    }
  }
  
  
  
  
  
  
  while (wait(NULL) > 0);

  for(int i =0; i<f_n; i++){
    close(f_k[i].pipe[0]);
    close(f_k[i].pipe[1]);
  }
  for(int i =0; i<s_n; i++){
    close(s_k[i].pipe[0]);
    close(s_k[i].pipe[1]);
  }
  fclose(franc);
  fclose(sarac);
  for(int i =0; i<f_n; i++){
    free(f_k[i].params);
  }
  for(int i =0; i<s_n; i++){
    free(s_k[i].params);
  }
  free(f_k);
  free(s_k);
  srand(time(NULL));
  printf("Opened descriptors: %d\n", count_descriptors());
}