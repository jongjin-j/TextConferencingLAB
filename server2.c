// #include <arpa/inet.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>

// #define PORT 8080
// #define MAX_CLIENTS 30
// #define BUFFER_SIZE 1024

// int main() {
//   int server_socket, client_sockets[MAX_CLIENTS], activity, max_sd, sd,
//       new_socket;
//   struct sockaddr_in server, client;
//   fd_set readfds;

//   // Create a socket
//   if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
//     perror("Socket creation failed");
//     exit(EXIT_FAILURE);
//   }

//   // Initialize server information
//   server.sin_family = AF_INET;
//   server.sin_addr.s_addr = INADDR_ANY;
//   server.sin_port = htons(PORT);

//   // Bind the socket
//   if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) == -1)
//   {
//     perror("Bind failed");
//     exit(EXIT_FAILURE);
//   }

//   // Listen for incoming connections
//   if (listen(server_socket, 5) == -1) {
//     perror("Listen failed");
//     exit(EXIT_FAILURE);
//   }

//   printf("Server listening on port %d\n", PORT);

//   // Accept incoming connections
//   int addrlen = sizeof(client);
//   char buffer[BUFFER_SIZE];
//   int client_count = 0;

//   // Initialize client_sockets array
//   for (int i = 0; i < MAX_CLIENTS; i++) {
//     client_sockets[i] = 0;
//   }

//   while (1) {
//     // Clear the socket set
//     FD_ZERO(&readfds);

//     // Add server socket to set
//     FD_SET(server_socket, &readfds);
//     max_sd = server_socket;

//     // Add child sockets to set
//     for (int i = 0; i < MAX_CLIENTS; i++) {
//       sd = client_sockets[i];

//       if (sd > 0) {
//         FD_SET(sd, &readfds);
//       }

//       if (sd > max_sd) {
//         max_sd = sd;
//       }
//     }

//     // Wait for activity on any of the sockets
//     activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

//     if (activity < 0) {
//       perror("Select error");
//     }

//     // If the server socket has activity, it's a new connection
//     if (FD_ISSET(server_socket, &readfds)) {
//       if ((new_socket = accept(server_socket, (struct sockaddr *)&client,
//                                (socklen_t *)&addrlen)) < 0) {
//         perror("Accept error");
//         exit(EXIT_FAILURE);
//       }

//       // Add the new socket to the array of client sockets
//       for (int i = 0; i < MAX_CLIENTS; i++) {
//         if (client_sockets[i] == 0) {
//           client_sockets[i] = new_socket;
//           printf("New connection, socket fd is %d, IP is : %s, port : %d\n",
//                  new_socket, inet_ntoa(client.sin_addr),
//                  ntohs(client.sin_port));
//           client_count++;
//           break;
//         }
//       }
//     }

//     // Check data from clients
//     for (int i = 0; i < MAX_CLIENTS; i++) {
//       sd = client_sockets[i];

//       if (FD_ISSET(sd, &readfds)) {
//         // Read data from the client
//         int valread = read(sd, buffer, BUFFER_SIZE);
//         if (valread == 0) {
//           // Client disconnected
//           getpeername(sd, (struct sockaddr *)&client, (socklen_t *)&addrlen);
//           printf("Host disconnected, IP %s, port %d\n",
//                  inet_ntoa(client.sin_addr), ntohs(client.sin_port));

//           // Close the socket and mark it as 0 in the array
//           close(sd);
//           client_sockets[i] = 0;
//           client_count--;
//         } else {
//           // Broadcast the message to other clients
//           for (int j = 0; j < MAX_CLIENTS; j++) {
//             if (client_sockets[j] != 0 && client_sockets[j] != sd) {
//               write(client_sockets[j], buffer, valread);
//             }
//           }
//         }
//       }
//     }
//   }

//   return 0;
// }
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define MAX_CLIENTS 30
#define BUFFER_SIZE 1024

int main() {
  int server_socket, client_sockets[MAX_CLIENTS], activity, max_sd, sd,
      new_socket;
  struct sockaddr_in server, client;
  fd_set readfds;

  // Create a socket
  if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Initialize server information
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(PORT);

  // Bind the socket
  if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) == -1) {
    perror("Bind failed");
    exit(EXIT_FAILURE);
  }

  // Listen for incoming connections
  if (listen(server_socket, 5) == -1) {
    perror("Listen failed");
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port %d\n", PORT);

  // Accept incoming connections
  int addrlen = sizeof(client);
  char buffer[BUFFER_SIZE];
  int client_count = 0;

  // Initialize client_sockets array
  for (int i = 0; i < MAX_CLIENTS; i++) {
    client_sockets[i] = 0;
  }

  while (1) {
    // Clear the socket set
    FD_ZERO(&readfds);

    // Add server socket to set
    FD_SET(server_socket, &readfds);
    max_sd = server_socket;

    // Add child sockets to set
    for (int i = 0; i < MAX_CLIENTS; i++) {
      sd = client_sockets[i];

      if (sd > 0) {
        FD_SET(sd, &readfds);
      }

      if (sd > max_sd) {
        max_sd = sd;
      }
    }

    // Wait for activity on any of the sockets
    activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

    if (activity < 0) {
      perror("Select error");
    }

    // If the server socket has activity, it's a new connection
    if (FD_ISSET(server_socket, &readfds)) {
      if ((new_socket = accept(server_socket, (struct sockaddr *)&client,
                               (socklen_t *)&addrlen)) < 0) {
        perror("Accept error");
        exit(EXIT_FAILURE);
      }

      // Add the new socket to the array of client sockets
      for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] == 0) {
          client_sockets[i] = new_socket;
          printf("New connection, socket fd is %d, IP is : %s, port : %d\n",
                 new_socket, inet_ntoa(client.sin_addr),
                 ntohs(client.sin_port));
          client_count++;
          break;
        }
      }
    }

    // Check data from clients
    for (int i = 0; i < MAX_CLIENTS; i++) {
      sd = client_sockets[i];

      if (FD_ISSET(sd, &readfds)) {
        // Read data from the client
        int valread = read(sd, buffer, BUFFER_SIZE);
        if (valread == 0) {
          // Client disconnected
          getpeername(sd, (struct sockaddr *)&client, (socklen_t *)&addrlen);
          printf("Host disconnected, IP %s, port %d\n",
                 inet_ntoa(client.sin_addr), ntohs(client.sin_port));

          // Close the socket and mark it as 0 in the array
          close(sd);
          client_sockets[i] = 0;
          client_count--;
        } else {
          // Broadcast the message to other clients
          for (int j = 0; j < MAX_CLIENTS; j++) {
            if (client_sockets[j] != 0 && client_sockets[j] != sd) {
              write(client_sockets[j], buffer, valread);
            }
          }
        }
      }
    }
  }

  return 0;
}
