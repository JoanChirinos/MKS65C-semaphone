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

// union semun {
//   int                 val;      /*  Value for SETVAL                */
//   struct semid_ds    *buf;      /*  Buffer for IPC_STAT, IPC_SET    */
//   unsigned short     *array;    /*  Array for GETALL, SETALL        */
//   struct seminfo     *__buf;    /*  Buffer for IPC_INFO             */
// };

int main() {
  // Get semaphore
  int sem_desc = semget(KEY, 1, 0);

  // Make sure no other operations are running
  struct sembuf* sbuf = calloc(sizeof(struct sembuf), 1);
  sbuf->sem_num = 0;
  sbuf->sem_op = -1;
  sbuf->sem_flg = SEM_UNDO;
  // This should wait until memory becomes available
  int sem_op_status = semop(sem_desc, sbuf, 1);

  int shmid = shmget(KEY, sizeof(int), 0644);
  if (shmid == -1) {
    printf("shmget error: %s\n", strerror(errno));
    return 1;
  }

  int fd = open("story.txt", O_RDWR | O_APPEND);
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
  printf("length of last line: %d\n", length);

  char* last_line = story_text + strlen(story_text) - length;

  printf("Last line in story:\n%s\n", last_line);

  printf("Your line:\n");
  char* new_line = calloc(sizeof(char), 2049);
  fgets(new_line, 2048, stdin);
  new_line[2047] = '\n';
  int new_line_length = strlen(new_line);
  printf("new line length: %d\n", new_line_length);

  // updates line length in shared memory
  data[0] = new_line_length;

  int write_status = write(fd, new_line, new_line_length);
  if (write_status == -1) {
    printf("write_error: %s\n", strerror(errno));
  }

  // upping the semaphore
  sbuf->sem_op = 1;
  semop(sem_desc, sbuf, 1);

}
