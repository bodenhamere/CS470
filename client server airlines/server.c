/**
 * Author: Emily Bodenhamer
 * Date: 6/8/20
 * Lab 5 CS 470
 **/
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// global variables
pthread_mutex_t myMutex;
int row = 2, col = 2, one = 1;
int **map;

// method headers
int *array();
int **matrix();
void free_mem();
void i_j(int connfd);
int check_matrix();
void print_matrix();
void ticket(int *mRow, int *mCol, int connfd);
void *program(void *connection);
void get_coordinates(int *mRow, int *mCol, int sockfd);

// methods
int main(int argc, char *argv[]) {
  int listenfd = 0, connfd = 0;
  struct sockaddr_in serv_addr;
  struct sockaddr_storage client_addr;

  // valid args
  if (argc > 2) {
    row = atoi(argv[1]);
    col = atoi(argv[2]);
  }

  // create ticket map
  map = matrix();

  // create socket
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&serv_addr, '0', sizeof(serv_addr));

  // define server address
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(4500);

  // tell the client what port to use
  printf("Please use port 4500 to connect to this server!\n");
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) < 0)
    perror("setsockopt(SO_REUSEADDR) failed");

  // bind socket to IP and port
  bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

  // listen for connections specify num of connections
  listen(listenfd, 10);

  int i = 0;
  pthread_t threads[10];
  pthread_mutex_init(&myMutex, NULL);
  while (1) {
    // manipulate the client side
    socklen_t len_client = sizeof(client_addr);
    connfd = accept(listenfd, (struct sockaddr *)&client_addr, &len_client);

    // begin program
    if (pthread_create(&threads[i], NULL, program, (void *)&connfd) != 0)
      printf("Failed to create thread\n");

    printf("Created thread for Client id %d\n", connfd);

    if (check_matrix(map, row, col) == 0) {
      break;
    }
    i++;
  }

  for (int i = 0; i < sizeof(threads); i++) {
    pthread_join(threads[i++], NULL);
  }
  return 0;
}

void *program(void *connection) {
  int connfd = *((int *)connection);
  char end[10], recBuff[100];
  int mRow, mCol;

  // read process
  read(connfd, &recBuff, sizeof(recBuff));
  char *ptr = strstr(recBuff, "automatic");

  while (1) {
    // print map
    printf("The matrix below is updated by Client id: %d\n", connfd);
    print_matrix();

    // check if the map is filled
    pthread_mutex_lock(&myMutex);
    if (check_matrix(map, row, col) == 0) {
      printf("The map is full. Thank you.\n");
      strcpy(end, "end");
      if (send(connfd, end, sizeof(end), 0) < 0) {
        printf("Client disconnected");
        exit(1);
      }
      break;
    } else {
      strcpy(end, "continue");
      send(connfd, end, sizeof(end), 0);
    }
    pthread_mutex_unlock(&myMutex);

    // receive end/con
    bzero(recBuff, strlen(recBuff));
    if (recv(connfd, recBuff, sizeof(recBuff), 0) < 0) {
      printf("Client disconnected");
      exit(1);
    }

    // send i and j
    i_j(connfd);

    // get ticket i and j
    get_coordinates(&mRow, &mCol, connfd);

    // get the tickets
    pthread_mutex_lock(&myMutex);
    ticket(&mRow, &mCol, connfd);
    pthread_mutex_unlock(&myMutex);

    if (ptr != NULL) { // auto
      sleep(1);
    } else {
      usleep(1000);
    }
  }
  pthread_mutex_unlock(&myMutex);
  close(connfd);
  pthread_exit(NULL);
}

void i_j(int connfd) {
  char server_message[256];
  bzero(server_message, strlen(server_message));
  // send a message
  snprintf(server_message, 200, "%d %d ,", row, col);
  send(connfd, server_message, sizeof(server_message), 0);
}

void get_coordinates(int *mRow, int *mCol, int sockfd) {
  char server_message[256];
  bzero(server_message, strlen(server_message));
  recv(sockfd, server_message, sizeof(server_message), 0);

  // extract i and j
  char *p = strtok(server_message, " ");
  *mRow = atoi(p);
  while (p != NULL) {
    p = strtok(NULL, " ");
    *mCol = atoi(p);
    break;
  }
}

void ticket(int *mRow, int *mCol, int connfd) {
  char server_message[256];
  bzero(server_message, strlen(server_message));
  if (map[*mRow][*mCol] != 0) {
    // send a message
    snprintf(
        server_message, 200,
        "This ticket from row: %d and column: %d has already been purchased",
        *mRow, *mCol);
    send(connfd, server_message, sizeof(server_message), 0);
  } else {
    map[*mRow][*mCol] = 1;
    snprintf(server_message, 200,
             "You have successfully purchased your ticket from row: %d and "
             "column: %d",
             *mRow, *mCol);
    send(connfd, server_message, sizeof(server_message), 0);
  }
}

int check_matrix() {
  for (int i = 0; i < row; i++) {
    for (int j = 0; j < col; j++) {
      if (map[i][j] == 0)
        return -1;
    }
  }
  return 0;
}

void print_matrix() {
  for (int i = 0; i < row; i++) {
    printf("| ");
    for (int j = 0; j < col; j++) {
      printf("%d ", map[i][j]);
    }
    printf("|\n");
  }
  printf("\n");
}

int **matrix() {
  int **matrix = (int **)calloc(row, sizeof(int *));
  for (int i = 0; i < row; i++) {
    matrix[i] = array();
  }

  // check if NULL
  if (matrix == NULL) {
    printf("Memory allocation error.");
    exit(0);
  }
  return matrix;
}

int *array() {
  int *arr = (int *)calloc(col, sizeof(int));

  // check if null
  if (arr == NULL) {
    printf("Memory allocation error.");
    exit(0);
  }
  return arr;
}