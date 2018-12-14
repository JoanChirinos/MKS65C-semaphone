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

// key
#define KEY 0xDAB42069

// struct
union semun {
  int                 val;      /*  Value for SETVAL                */
  struct semid_ds    *buf;      /*  Buffer for IPC_STAT, IPC_SET    */
  unsigned short     *array;    /*  Array for GETALL, SETALL        */
  struct seminfo     *__buf;    /*  Buffer for IPC_INFO             */
};

// create(-c)
int create_game() {
  // Making shared memory
  int shmid = shmget(KEY, sizeof(int), 0644 | IPC_CREAT);
  if (shmid == -1) {
    printf("Error: %s\n", strerror(errno));
    return 1;
  }

  // Making semaphore
  int sem_desc = semget(KEY, 1, IPC_CREAT | IPC_EXCL | 0644);
  if (sem_desc == -1) {
    printf("Error: %s\n", strerror(errno));
    return 1;
  }
  // set semaphore value
  union semun sem_data;
  sem_data.val = 1;
  int semctl_status = semctl(sem_desc, 0, SETVAL, sem_data);
  if (semctl_status == -1) {
    printf("semctl error: %s\n", strerror(errno));
  }

  // Making file
  int fd = open("story.txt", O_CREAT | O_TRUNC, 0644);
  if (fd == -1) {
    printf("Error: %s\n", strerror(errno));
    return 1;
  }
  return 0;
}

// remove(-r) will remove text file and print the story
int remove_game() {
  // Get semaphore
  int sem_desc = semget(KEY, 1, 0);

  // Make sure no other operations are running
  struct sembuf* sbuf = calloc(sizeof(struct sembuf), 1);
  sbuf->sem_num = 0;
  sbuf->sem_op = -1;
  sbuf->sem_flg = SEM_UNDO;
  // This should wait until memory becomes available
  int sem_op_status = semop(sem_desc, sbuf, 1);
  free(sbuf);

  // Displaying story
  int fd = open("story.txt", O_RDONLY);
  char* story_text = calloc(sizeof(char), 20480);
  read(fd, story_text, 20479);
  printf("Story:\n%s\n", story_text);
  free(story_text);

  // Removing shared memory
  int shmid = shmget(KEY, sizeof(int), 0);
  int shmget_status = shmctl(shmid, IPC_RMID, NULL);
  if (shmget_status == -1) {
    printf("Error: %s\n", strerror(errno));
    return 1;
  }

  // Removing semaphore
  int semctl_status = semctl(sem_desc, 0, IPC_RMID, NULL);
  if (semctl_status == -1) {
    printf("Error: %s\n", strerror(errno));
    return 1;
  }

  // Removing file
  int remove_status = remove("story.txt");
  if (remove_status == -1) {
    printf("Error: %s\n", strerror(errno));
    return 1;
  }

  return 0;
}

//  verbose(-v) will print the contents of the story.txt file
int view_story() {
  // Displaying story
  int fd = open("story.txt", O_RDONLY);
  char* story_text = calloc(sizeof(char), 20480);
  int status = read(fd, story_text, 20479);
  if (status == -1) {
    printf("Error: %s\n", strerror(errno));
    return 1;
  }
  printf("Story:\n%s\n", story_text);
  free(story_text);
  return 0;
}

int main(int argc, char* argv[]) {
  if (argc == 1) {
    printf("You need the flags -c, -r, or -v\n");
    return 1;
  }
  if (!strcmp(argv[1], "-c")) {
    if (create_game()) { return 1; }
  }
  else if (!strcmp(argv[1], "-r")) {
    if (remove_game()) { return 1; }
  }
  else if (!strcmp(argv[1], "-v")) {
    if (view_story()) { return 1; }
  }
  else {
    printf("You need the flags -c, -r, or -v\n");
    return 1;
  }
  return 0;
}
