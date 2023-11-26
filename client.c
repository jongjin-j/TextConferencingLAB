// #include <arpa/inet.h> // inet_addr()
// #include <netdb.h>
// #include <stdbool.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <strings.h> // bzero()
// #include <sys/socket.h>
// #include <unistd.h> // read(), write(), close()
// #define MAX 80
// #define PORT 8040
// #define SA struct sockaddr

// // Wait for user input
// char userInput[100];
// bool welcomed = false;

// void welcome();

// void func(int sockfd) {
//   char buff[MAX];
//   int n;
//   for (;;) {
//     if ((strncmp(buff, "quit", 4)) == 0) {
//       break;
//     }

//     if ((strncmp(buff, "logout", 6)) == 0) {
//       break;
//     }

//     bzero(buff, sizeof(buff));
//     printf("Enter command : ");
//     n = 0;
//     while ((buff[n++] = getchar()) != '\n')
//       ;
//     write(sockfd, buff, sizeof(buff));

//     bzero(buff, sizeof(buff));
//     read(sockfd, buff, sizeof(buff));

//     if ((strncmp(buff, "Welcome", 7)) == 0 && welcomed == false) {
//       printf("Login successful : %s\n", buff);
//       welcomed = true;
//       bzero(buff, sizeof(buff));
//     } else {
//       printf("%s\n", buff);
//       bzero(buff, sizeof(buff));
//     }
//   }
// }

// int main() {
//   welcome();
//   int sockfd, connfd;
//   struct sockaddr_in servaddr, cli;

//   // socket create and verification
//   sockfd = socket(AF_INET, SOCK_STREAM, 0);
//   if (sockfd == -1) {
//     printf("socket creation failed...\n");
//     exit(0);
//   } else
//     // printf("Socket successfully created..\n");
//     bzero(&servaddr, sizeof(servaddr));

//   // assign IP, PORT
//   servaddr.sin_family = AF_INET;
//   servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
//   servaddr.sin_port = htons(PORT);

//   // connect the client socket to server socket
//   if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0) {
//     printf("connection with the server failed...\n");
//     exit(0);
//   } else
//     // first = false;
//     // printf("connected to the server..\n");

//     // function for chat
//     func(sockfd);

//   // close the socket
//   close(sockfd);
// }

// void welcome() {
//   printf("Welcome to Multi-Party Text Conferencing Program!\n");
//   printf("====================================================================="
//          "============\n");
//   printf("Available Commands:\n");
//   printf("1. /login <username> <password> <IP> <port>    - Log in with your "
//          "credentials\n");
//   printf("2. /logout                                     - Exit the
//   server\n"); printf("3. /join <session_id>                          - Join a
//   specific "
//          "session\n");
//   printf("4. /leavesession                               - Leave the current
//   "
//          "session\n");
//   printf(
//       "5. /createsession <session_id>                 - Create a new session
//       " "and join it\n");
//   printf("6. /list                                       - List connected "
//          "clients and available sessions\n");
//   printf("7. /quit                                       - Terminate the "
//          "program\n");
//   printf("8. <text>                                      - Send a message to
//   "
//          "the current conference session\n");
//   printf("====================================================================="
//          "============\n");

//   // // Prompt the user to enter something
//   // printf("Enter a command: ");

//   // scanf("%99s", userInput);
//   return;
// }

#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8040
#define BUFFER_SIZE 1024

void welcome();

void *receiveMessages(void *socket_desc) {
  int client_socket = *(int *)socket_desc;
  char buffer[BUFFER_SIZE];

  while (1) {
    memset(buffer, 0, sizeof(buffer));

    // Receive and display the server's response
    if (read(client_socket, buffer, BUFFER_SIZE) > 0) {
      printf("\nServer says: %s\n", buffer);
      memset(buffer, 0, BUFFER_SIZE);
    }
  }
}

int main() {
  int client_socket;
  struct sockaddr_in server;
  char buffer[BUFFER_SIZE];

  // Create a socket
  if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Initialize server information
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  server.sin_port = htons(PORT);

  // Connect to server
  if (connect(client_socket, (struct sockaddr *)&server, sizeof(server)) ==
      -1) {
    perror("Connection failed");
    exit(EXIT_FAILURE);
  }

  printf("Connected to server\n");

  // Create a thread for receiving messages
  pthread_t recv_thread;
  if (pthread_create(&recv_thread, NULL, receiveMessages,
                     (void *)&client_socket) != 0) {
    perror("Thread creation failed");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Get input from user
    printf("Enter message: ");
    fgets(buffer, BUFFER_SIZE, stdin);

    // Send the message to the server
    write(client_socket, buffer, strlen(buffer));

    // Clear the buffer
    memset(buffer, 0, BUFFER_SIZE);
  }

  // Close the socket
  close(client_socket);

  return 0;
}

void welcome() {
  printf("Welcome to Multi-Party Text Conferencing Program!\n");
  printf("====================================================================="
         "============\n");
  printf("Available Commands:\n");
  printf("1. /login <username> <password> <IP> <port>    - Log in with your "
         "credentials\n");
  printf("2. /logout  - Exit the server\n");
  printf(" 3. / join<session_id> - Join a specific session\n");
  printf("4. /leavesession - Leave the current session\n");
  printf(
      "5. /createsession <session_id>  - Create a new session and join it\n");
  printf("6. /list                                       - List connected "
         "clients and available sessions\n");
  printf("7. /quit                                       - Terminate the "
         "program\n");
  printf("8. <text>                                      - Send a message to "
         "the current conference session\n");
  printf("====================================================================="
         "============\n");

  // // Prompt the user to enter something
  // printf("Enter a command: ");

  // scanf("%99s", userInput);
  return;
}
