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

#define MAX_USERS 4
#define MAX_USERNAME_LENGTH 80
#define MAX_PASSWORD_LENGTH 80
#define PORT 8040
#define SA struct sockaddr
#define MAX_NAME 80
#define MAX_DATA 80

#define LOGIN 1
#define LO_ACK 2
#define LO_NAK 3
#define EXIT 4
#define JOIN 5
#define JN_ACK 6
#define JN_NAK 7
#define LEAVE_SESS 8
#define NEW_SESS 9
#define NS_ACK 10
#define MESSAGE 11
#define QUERY 12
#define QU_ACK 13
#define LOGOUT 14

struct message {
  unsigned int type;
  unsigned int size;
  unsigned char source[MAX_NAME];
  unsigned char data[MAX_DATA];
};

//-----------SESSION DATA STRUCTURES------/
#define MAX_CLIENTS 3
int numberOfSessions = 0;
struct Session {
  char session_ID[MAX];
  int clients[MAX_CLIENTS];
  int numClientsInSession;
};
// struct Session *sessionArray = NULL;
struct Session sessionArray[30];
//-----------CLIENT DATA STRUCTURES------/
#define MAX_CLIENTS 3
int numberOfClients = 0;
struct Client {
  char client_ID[MAX];
  int session_ID;
  char sessionName[MAX];
  int ip_addr;
  int port_addr;
  bool loggedIn;
};
struct Client clientArray[30];

//-----------USER DATA STRUCTURES------/
struct User {
  char username[MAX_USERNAME_LENGTH];
  char password[MAX_USERNAME_LENGTH];
  bool loggedIn;
};

//-----------HARDCODED USER DATA------/
struct User users[MAX_USERS] = {
    {"jill", "eW94dsol"},
    {"jack", "432wlFd"},
    {"user3", "pass3"},
    {"user4", "pass4"},
};

bool first = false;

void messageFormer(struct message *message, char *buffer) {
  // format "type:size:source:data"
  char *token = strtok(buffer, ":");
  printf("token %s", token);
  message->type = atoi(token);
  token = strtok(NULL, ":");
  printf("token %s", token);
  message->size = atoi(token);
  token = strtok(NULL, ":");
  printf("token %s", token);
  strcpy(message->source, token);
  token = strtok(NULL, ":");
  printf("token %s", token);
  strcpy(message->data, token);
}

// Driver function
int main(int argc, char *argv[]) {
  // new variables
  int opt = true;
  int master_socket, addrlen, new_socket, max_clients = 30, activity, i,
                                          valread, sd;

  int client_socket[30];

  for (int i = 0; i < 30; i++) {
    client_socket[i] = -1;
  }

  int max_sd;
  struct sockaddr_in address;

  char buffer[1025]; // data buffer of 1k

  // set of socket descriptrs
  fd_set readfds;

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

      puts("Welcome message sent successfully");
      first = true;

      // add new socket to array of sockets
      for (int i = 0; i < max_clients; i++) {
        // if position is empty
        if (client_socket[i] == -1) {
          client_socket[i] = new_socket;
          printf("Adding to list of sockets as %d\n", i);
          // numberOfClients++;
          break;
        }
      }
    } else {

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
            close(sd);
            client_socket[i] = 0;
          }
          // echo back message that came in
          else {
            buffer[valread] = '\0';

            printf("Message from client %d: %s", i, buffer);
            int currClient = i;

            struct message message;
            messageFormer(&message, buffer);
            //printf("DATA %s\n", message.data);

            //-------------------LOG
            // IN----------------------------------------------------
            if (strncmp("/login", message.data, strlen("/login")) == 0) {
              // will need to perform some error checking in here
              // for things like correct types, handle cases for missing
              // arguments, etc.
              printf("\nCOMMAND: LOGIN...\n");
              char *token = strtok(message.data, " "); // assuming space as delimter
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

              //----------------------------authenticate user
              int authenticate = 0;
              int currUser = -1;
              bool breakLogin = false;

              if (breakLogin == false) {
                for (int i = 0; i < MAX_USERS; i++) {
                  if (strcmp(users[i].username, client_ID) == 0) {
                    if (strcmp(users[i].password, password) == 0) {
                      printf("Authentication successful.\n");
                      numberOfClients++;
                      currUser = i;

                      // clientArray = (struct Client *)malloc((numberOfClients
                      // + 1) *
                      //                                       sizeof(struct
                      //                                       Client));

                      strcpy(clientArray[numberOfClients - 1].client_ID,
                             client_ID);
                      clientArray[numberOfClients - 1].session_ID =
                          -1; // no sessions created yet

                      authenticate = 1;
                    }
                  }
                }
                if (clientArray[i].loggedIn == true) {
                  char *auth = "Please log out and try again.";
                  send(sd, auth, strlen(auth), 0);
                  breakLogin = true;
                  break;
                }
                if (authenticate == 0) {
                  char *auth = "Authentication failed. Please try again.";
                  send(sd, auth, strlen(auth), 0);
                  break;
                }
                if (currUser != -1) {
                  if (users[currUser].loggedIn == true) {
                    char welcomeMessage[] =
                        "%s is already logged in to session.";
                    char welcomeBuffer[256];
                    snprintf(welcomeBuffer, sizeof(welcomeBuffer),
                             welcomeMessage, users[currUser].username);
                    send(sd, welcomeBuffer, strlen(welcomeBuffer), 0);
                    break;
                  } else {
                    clientArray[numberOfClients - 1].loggedIn = true;
                    char welcomeMessage[] = "Welcome, %s!";
                    char welcomeBuffer[256];
                    snprintf(welcomeBuffer, sizeof(welcomeBuffer),
                             welcomeMessage, users[currUser].username);
                    send(sd, welcomeBuffer, strlen(welcomeBuffer), 0);
                    users[currUser].loggedIn = true;
                    break;
                  }
                }
              }
            }
            //------------------------------------------------------------------------------
            //-------------------CREATE
            // SESSION----------------------------------------------------
            if (strncmp("/createsession", buffer, strlen("/createsession")) ==
                0) {
              // will need to perform some error checking in here
              // for things like correct types, handle cases for missing
              // arguments, etc.
              printf("\nCOMMAND: CREATE_SESSION...\n");
              char *token = strtok(buffer, " "); // assuming space as delimter
              token = strtok(NULL, " ");         // get second argument
              printf("session name is: %s\n", token);
              char *session_name = token;
              numberOfSessions++;
              // sessionArray = (struct Session *)malloc((numberOfSessions) *
              //                                             sizeof(struct
              //                                             Session));
              sessionArray[numberOfSessions - 1].numClientsInSession = 0;
              strcpy(sessionArray[numberOfSessions - 1].session_ID, token);
            }
            //------------------------------------------------------------------------------
            //-------------------LIST----------------------------------------------------
            if (strncmp("/list", buffer, strlen("/list")) == 0) {
              // will need to perform some error checking in here
              // for things like correct types, handle cases for missing
              // arguments, etc.
              char message[500]; // Adjust the size according to your needs

              // Concatenate messages to the string
              printf(message, "\nCOMMAND: LIST...\n");

              sprintf(message + strlen(message), "\nAvailable sessions:\n");
              for (int i = 0; i < numberOfSessions; i++) {
                sprintf(message + strlen(message), "%s",
                        sessionArray[i].session_ID);
              }
              sprintf(message + strlen(message), "Connected clients:\n");
              // sprintf(message + strlen(message), "num of clients: %d\n",
              //         numberOfClients);
              for (int i = 0; i < numberOfClients; i++) {
                sprintf(message + strlen(message), "%s\n",
                        clientArray[i].client_ID);
              }

              // Now, you can send the message using your socket
              // Assuming sd is the socket descriptor.
              // Note: Ensure that the socket is properly set up before sending.

              send(sd, message, strlen(message), 0);

              // printf("\nCOMMAND: LIST...\n");
              // printf("Available sessions:\n\n");
              // for (int i = 0; i < numberOfSessions; i++) {
              //   printf("%s", sessionArray[i].session_ID);
              // }
              // printf("Connected clients:\n\n");
              // printf("num of clients: %d\n", numberOfClients);
              // printf("%s\n", clientArray[0].client_ID);
              // char *message2 = "fill in here";
              // send(sd, message2, strlen(message2), 0);
            }
            //------------------------------------------------------------------------------
            //-------------------Join
            // session----------------------------------------------------
            if (strncmp("/joinsession", buffer, strlen("/joinsession")) == 0) {
              // will need to perform some error checking in here
              // for things like correct types, handle cases for missing
              // arguments, etc.
              printf("\nCOMMAND: JOIN_SESSION...\n");
              char *token = strtok(buffer, " "); // assuming space as delimter
              token = strtok(NULL, " ");         // get second argument
              printf("joining %s session...\n", token);
              char *session_name = token;

              int currSession;
              // sanity check
              for (int i = 0; i < numberOfSessions; i++) {
                if (strcmp(sessionArray[i].session_ID, token) == 0) {
                  printf("Session exists...\n");
                  currSession = i;
                  int indice = sessionArray[i].numClientsInSession;
                  // increase the number of clients in the session
                  sessionArray[i].numClientsInSession++;
                } else {
                  printf("ERROR!!\n");
                }
              }

              for (int i = 0; i < numberOfClients; i++) {
                if (i == currClient) {
                  clientArray[i].session_ID = currSession;
                  strcpy(clientArray[i].sessionName, token);
                  printf("client %d is in session %s\n", i, token);
                }
              }

              // and notify all clients
              for (int i = 0; i < numberOfClients; i++) {
                if (clientArray[i].session_ID == currSession) {
                  char *message2 = "hows it going!!...\n";
                  send(sd, message2, strlen(message2), 0);
                }
              }
            }
            //------------------------------------------------------------------------------
            //-------------------TEXT----------------------------------------------------
            if (strncmp("/", buffer, strlen("/")) != 0) {
              // will need to perform some error checking in here
              // for things like correct types, handle cases for missing
              // arguments, etc.

              // get current session name
              char currSessionName[50];
              for (int i = 0; i < numberOfClients; i++) {
                if (i == currClient) {
                  strcpy(currSessionName, clientArray[i].sessionName);
                }
              }

              printf("\nCOMMAND: TEXT...\n");

              printf("message is: %s and its coming from client %d in session "
                     "%s\n",
                     buffer, i, currSessionName);

              // send to all clients in the session
              for (int i = 0; i < numberOfSessions; i++) {
                if (strcmp(sessionArray[i].session_ID, currSessionName) == 0) {
                  // found the session
                  for (int j = 0; j < sessionArray[i].numClientsInSession;
                       j++) {
                    if (j != currClient) {
                      printf("%d is in session\n", j);
                      sd = client_socket[j];
                      send(sd, buffer, strlen(buffer), 0);
                      // close(sd2);
                    }
                  }
                }
              }
            }
            //-------------------LOG
            // OUT---------------------------------------------------
            if (strncmp("/logout", buffer, strlen("/list")) == 0) {
              int currClient = i;

              // will need to perform some error checking in here
              // for things like correct types, handle cases for missing
              // arguments, etc.
              printf("\nCOMMAND: LOGOUT...\n");
              for (int i = 0; i < 30; i++) {
                sd = client_socket[i];
                clientArray[i].loggedIn = false;
                users[i].loggedIn = false;
                char *logout = "logout";
                send(sd, logout, strlen(logout), 0);
              }
            }
            //------------------------------------------------------------------------------
            //-------------------QUIT---------------------------------------------------
            if (strncmp("/quit", buffer, strlen("/quit")) == 0) {
              // will need to perform some error checking in here
              // for things like correct types, handle cases for missing
              // arguments, etc.
              printf("\nCOMMAND: QUIT...\n");
              printf("terminating the program...\n");

              for (int i = 0; i < 30; i++) {
                sd = client_socket[i];
                char *quit = "quit";
                send(sd, quit, strlen(quit), 0);
              }
              close(master_socket);
              return 0;
            }
            //------------------------------------------------------------------------------
            //------------------------------------------------------------------------------
            if (currClient == 0) {
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
  }
  return 0;
}