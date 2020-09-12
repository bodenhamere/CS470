/**
 * Author: Emily Bodenhamer
 * Date: 6/8/20
 * Lab 5 CS 470
 **/
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// method headers
void get_sizes(int *row, int *col, int sockfd);
void get_coordinates(int *row, int *col, int type, int sockfd);
int valid_num(char cRow[], char cCol[], int *row, int *col);
void validity(int argc, char *argv[]);

// methods
int main(int argc, char *argv[]) {
  srand(time(NULL));
  int sockfd = 0, row, col;
  struct sockaddr_in serv_addr;
  char server_reply[500], process[100], endPro[10];

  // check if null
  if (argc != 4) {
    printf(
        "You must enter automatic or manual, an ip address and port number\n");
    exit(1);
  }
  // validate args
  validity(argc, argv);

  // creates the client socket
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("\n Error : Could not create socket \n");
    return 1;
  }

  // specify fam and port num
  memset(&serv_addr, '0', sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(atoi(argv[3]));
  if (inet_pton(AF_INET, argv[2], &serv_addr.sin_addr) <= 0) {
    printf("Bad address\n");
    return 1;
  }
  
  // connect to the socket
  for (int times = 0; times < 10; times++) {
    printf("Server not available, retrying..\n");
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) >=
        0) {
      break;
    } else if (times == 9) {
      printf("Exiting.. the server is not running.\n");
      exit(1);
    }
  }

  // send mode
  strcpy(process, argv[1]);
  printf("You are now connected to the server.\n");
  if (write(sockfd, &process, sizeof(process)) < 0) {
    printf("Invalid mode, exiting..\n");
    exit(1);
  }
  char *ptr = strstr(argv[1], "automatic"), *end;

  while (1) {
    // check if matrix is filled
    bzero(endPro, strlen(endPro));
    recv(sockfd, endPro, sizeof(endPro), 0);
    end = strstr(endPro, "end");
    if (end != NULL) {
      printf("The map is full. Exiting..\n");
      break;
    }

    send(sockfd, "Got it.", 10, 0);
    // get the size of the matrix
    get_sizes(&row, &col, sockfd);

    if (ptr != NULL) { // auto
      printf("Random tickets will be selected!\n\n");
      // send row and col
      get_coordinates(&row, &col, 1, sockfd);

      // receive row and col from server
      bzero(server_reply, strlen(server_reply));
      recv(sockfd, server_reply, sizeof(server_reply), 0);
      printf("%s\n ", server_reply);
    } else {
      printf("Please manually purchase your tickets!\n\n");
      // send row and col
      get_coordinates(&row, &col, 2, sockfd);

      // receive row and col from server
      bzero(server_reply, strlen(server_reply));
      recv(sockfd, server_reply, sizeof(server_reply), 0);
      printf("%s\n ", server_reply);
    }
  }
  // close socket
  close(sockfd);
  return 0;
}

void get_sizes(int *row, int *col, int sockfd) {
  char server_message[256];
  bzero(server_message, strlen(server_message));
  recv(sockfd, server_message, sizeof(server_message), 0);

  // extract i and j
  char *p = strtok(server_message, " ");
  *row = atoi(p);
  while (p != NULL) {
    p = strtok(NULL, " ");
    *col = atoi(p);
    break;
  }
  printf("The ticket map is an %dx%d matrix\n\n", *row, *col);
}

void get_coordinates(int *row, int *col, int type, int sockfd) {
  int mRow, mCol;
  char server_message[256];
  bzero(server_message, strlen(server_message));
  if (type == 1) {        // auto
    mRow = rand() % *row; // i = row
    mCol = rand() % *col; // j = col

    printf("You've randomly chosen %d as i and %d as j\n\n", mRow, mCol);
    // send a message
    snprintf(server_message, 200, "%d %d ,", mRow, mCol);
    send(sockfd, server_message, sizeof(server_message), 0);
  } else {
    char cRow[100], cCol[100];

    while (1) {
      printf("Please enter what row you would like: ");
      fgets(cRow, 100, stdin);
      printf("\nPlease enter what column you would like: ");
      fgets(cCol, 100, stdin);

      // error handeling input
      if (valid_num(cRow, cCol, row, col) == 0)
        break;
    }

    // send a message
    snprintf(server_message, 200, "%d %d ,", atoi(cRow), atoi(cCol));
    send(sockfd, server_message, sizeof(server_message), 0);
  }
}

int valid_num(char cRow[], char cCol[], int *row, int *col) {
  char *endptr1, *endptr2;
  int base = 10;
  strtol(cRow, &endptr1, base);
  strtol(cCol, &endptr2, base);

  if (*endptr1 == '\0' || endptr1 == cRow || *endptr2 == '\0' ||
      endptr2 == cRow) {
    printf("Your input contains non-numbers\n");
    return -1;
  } else {
    // error check if its within range
    if (atoi(cRow) >= *row || atoi(cCol) >= *col) {
      printf("Your input was out of the maps range.\n");
      return -1;
    } else {
      return 0;
    }
  }
  return 0;
}

void validity(int argc, char *argv[]) {
  // check which processing mode to use
  char *autom = strstr(argv[1], "automatic");
  char *man = strstr(argv[1], "manual");
  if (autom != NULL || man != NULL) {
    printf("You've chosen %s\n\n", argv[1]);
  } else {
    printf("Please enter a processing mode, automatic or manual.\n");
    exit(1);
  }
}