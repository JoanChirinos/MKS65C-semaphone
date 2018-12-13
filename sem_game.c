#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

//  semaphore stuff
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define KEY 0xDAB42069

int main() {
  // Get semaphore
  int sem_desc = semget(KEY, 1, 0);

  // Make sure no other operations are running
  struct sembuf* sbuf = calloc(sizeof(struct sembuf), 1);
  sbuf->sem_num = 0;
  sbuf->sem_op = -1;
  sbuf->sem_flg = 0;
  // This should wait until memory becomes available
  int sem_op_status = semop(sem_desc, sbuf, 1);

  int shmid = shmget(KEY, sizeof(int), 0644);
  if (shmid == -1) {
    printf("shmget error: %s\n", strerror(errno));
    return 1;
  }

  int fd = open("story.txt", O_WRONLY | O_APPEND);
  if (fd == -1) {
    printf("open error: %s\n", strerror(errno));
  }
  char* story_text = calloc(sizeof(char), 20480);
  int status = read(fd, story_text, 20479);
  if (status == -1) {
    printf("read error: %s\n", strerror(errno));
    return 1;
  }

  int* data = shmat(shmid, (void*)0, 0);
  int length = data[0];

  char* last_line = story_text + lseek(fd, length, SEEK_END);

  printf("Last line in story:\n%s\n", last_line);

  printf("Your line:\n");
  char* new_line = calloc(sizeof(char), 2049);
  fgets(new_line, 2048, stdin);
  new_line[2047] = '\n';
  int new_line_length = strlen(new_line);
  printf("new line length: %d\n", new_line_length);

  union semun* sem_data = calloc(sizeof(union semun), 1);
  sem_data->val = new_line_length;

  int semctl_status = semctl(sem_desc, 0, SETVAL, sem_data);
  if (semctl_status == -1) {
    printf("%s\n", strerror(errno));
  }

  int write_status = write(fd, new_line, new_line_length);
  if (write_status == -1) {
    printf("%s\n", strerror(errno));
  }

}
