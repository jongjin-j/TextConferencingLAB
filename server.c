// source for TCP:
// https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/ source
// for multi clients:
// https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/

// note: a user is someone registered in the database, we have their login
// credentials a client is someone trying to authenticate, ie. a user who is
// active right now

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()
#define MAX 80

#define MAX_USERS 3
#define MAX_USERNAME_LENGTH 80
#define MAX_PASSWORD_LENGTH 80
#define PORT 8040
#define SA struct sockaddr

//-----------SESSION DATA STRUCTURES------/
#define MAX_CLIENTS 3
int numberOfSessions = 0;
struct Session {
  int session_ID;
  int clients[MAX_CLIENTS];
  int numClientsInSession;
};
struct Session *sessionArray = NULL;

//-----------CLIENT DATA STRUCTURES------/
#define MAX_CLIENTS 3
int numberOfClients = 0;
struct Client {
  char client_ID[MAX];
  int session_ID;
  int ip_addr;
  int port_addr;
};
struct Client *clientArray = NULL;

//-----------USER DATA STRUCTURES------/
struct User {
  char username[MAX_USERNAME_LENGTH];
  char password[MAX_USERNAME_LENGTH];
};

//-----------HARDCODED USER DATA------/
struct User users[MAX_USERS] = {
    {"jill", "eW94dsol"},
    {"jack", "432wlFd"},
    {"user3", "pass3"},
};

bool first = false;

// Driver function
int main(int argc, char *argv[]) {
  // new variables
  int opt = true;
  int master_socket, addrlen, new_socket, client_socket[30],
      max_clients = 30, activity, i, valread, sd;

  int max_sd;
  struct sockaddr_in address;

  char buffer[1025]; // data buffer of 1k

  // set of socket descriptrs
  fd_set readfds;

  for (int i = 0; i < MAX_CLIENTS; i++) {
    client_socket[i] = 0;
  }

  int sockfd, connfd, len;

  // create a master socket
  if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // set master socket to allow multiple connections
  //  good habit, program would still work without this tho
  if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                 sizeof(opt)) < 0) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  // types of socket created
  // assign IP, PORT
  address.sin_family = AF_INET;
  // servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  // bind the socket to localhost port 8030
  if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }
  printf("Listener on port %d \n", PORT);

  // try to specify maximum of 3 pending connections for the master socket
  if (listen(master_socket, 3) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  // accept the incoming connection
  addrlen = sizeof(address);
  puts("Waiting for connections...");

  while (true) {
    // clear the socket set
    FD_ZERO(&readfds);

    // add master socket to the set
    FD_SET(master_socket, &readfds);
    max_sd = master_socket;

    // add child socket to set
    for (i = 0; i < max_clients; i++) {
      // socket descriptor
      sd = client_socket[i];
      // if valid socket descriptor then add to read list
      if (sd > 0) {
        FD_SET(sd, &readfds);
      }
      // highest file descriptor number, need it for the select function
      if (sd > max_sd) {
        max_sd = sd;
      }
    }
    // wait for an activity on one of the sockets, timeour is NULL, so wait
    // indefinitely
    activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

    if ((activity < 0) && (errno != EINTR)) {
      printf("Select error!");
    }

    // if something happened on the master socket, then its an incoming
    // connection
    if (FD_ISSET(master_socket, &readfds)) {
      if ((new_socket = accept(master_socket, (struct sockaddr *)&address,
                               (socklen_t *)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
      }

      // inform user of socket number - used in send and receive commands
      printf("New connection, socket fd is %d, ip is %s, port %d\n", new_socket,
             inet_ntoa(address.sin_addr), ntohs(address.sin_port));

      // send new connection greeting message

      char *message3 = "new message...\n";
      if (send(new_socket, message3, strlen(message3), 0) != strlen(message3)) {
        perror("Send");
      }

      puts("Welcome message sent successfully");
      first = true;

      // add new socket to array of sockets
      for (int i = 0; i < max_clients; i++) {
        // if position is empty
        if (client_socket[i] == 0) {
          client_socket[i] = new_socket;
          printf("Adding to list of sockets as %d\n", i);
          break;
        }
      }
    }

    for (i = 0; i < max_clients; i++) {

      sd = client_socket[i];
      close(sd);
    }

    // else it has some I/O operation on some other socket
    for (i = 0; i < max_clients; i++) {

      sd = client_socket[i];

      if (FD_ISSET(sd, &readfds)) {

        // check if it was for closing, and also read the incoming message
        if ((valread = read(sd, buffer, 1024)) == 0) {
          // somebody disconnected, get details and print
          getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
          printf("Host disconnected, ip %s, port %d \n",
                 inet_ntoa(address.sin_addr), ntohs(address.sin_port));

          // close the socket and mark as 0 in list for reuse

          client_socket[i] = 0;
        }
        // echo back message that came in
        else {
          buffer[valread] = '\0';

          printf("Message from client %d: %s", i, buffer);

          if (strncmp("/login", buffer, strlen("/login")) == 0) {
            // will need to perform some error checking in here
            // for things like correct types, handle cases for missing
            // arguments, etc.
            printf("COMMAND: LOGIN...\n");
            char *token = strtok(buffer, " "); // assuming space as delimter
            token = strtok(NULL, " ");         // get second argument
            printf("client_ID is: %s\n", token);
            char *client_ID = token;

            token = strtok(NULL, " "); // get third argument
            printf("password is: %s\n", token);
            char *password = token;

            token = strtok(NULL, " "); // get fourth argument
            printf("ip addr is: %s\n", token);
            char *addr = token;

            token = strtok(NULL, " "); // get fifth argument
            printf("port is: %s\n", token);
            char *port = token;

            //-------------authenticate user
            int authenticate = 0;
            for (int i = 0; i < MAX_USERS; i++) {
              if (strcmp(users[i].username, client_ID) == 0) {
                if (strcmp(users[i].password, password) == 0) {
                  printf("Authentication successful.\n");

                  clientArray = (struct Client *)malloc((numberOfClients + 1) *
                                                        sizeof(struct Client));
                  strcpy(clientArray[numberOfClients].client_ID, client_ID);
                  clientArray[numberOfClients].session_ID =
                      -1; // no sessions created yet
                  numberOfClients++;

                  authenticate = 1;
                }
              }
            }
            if (authenticate == 0) {
              printf("Authentication failed!\n");
            } else {
              char *auth = "Authentication successful!\n";
              send(sd, auth, strlen(auth), 0);
              break;
            }

            for (int i = 0; i < numberOfClients; i++) {
              printf("%s\n", clientArray[i].client_ID);
            }
          }

          if (i == 0) {
            char *message2 = "hey client 0, its you!!...\n";
            send(sd, message2, strlen(message2), 0);
            break;
          } else {
            char *message2 = "you're client 1, lets go!!...\n";
            send(sd, message2, strlen(message2), 0);
            break;
          }
        }
      }
    }
  }
  return 0;
}
