// server.c
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 12345
#define MAX_CONNECTIONS 5
#define BUFFER_SIZE 1024

void *handle_client(void *arg);

int main() {
  int server_socket, client_socket;
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  pthread_t tid;

  // Create server socket
  if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Configure server address
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  // Bind the server socket
  if (bind(server_socket, (struct sockaddr *)&server_addr,
           sizeof(server_addr)) == -1) {
    perror("Bind failed");
    exit(EXIT_FAILURE);
  }

  // Listen for incoming connections
  if (listen(server_socket, MAX_CONNECTIONS) == -1) {
    perror("Listen failed");
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port %d...\n", PORT);

  while (1) {
    // Accept a client connection
    if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr,
                                &client_addr_len)) == -1) {
      perror("Accept failed");
      exit(EXIT_FAILURE);
    }

    printf("Client connected: %s\n", inet_ntoa(client_addr.sin_addr));

    // Create a thread to handle the client
    if (pthread_create(&tid, NULL, handle_client, (void *)&client_socket) !=
        0) {
      perror("Thread creation failed");
      exit(EXIT_FAILURE);
    }
  }

  close(server_socket);
  return 0;
}

void *handle_client(void *arg) {
  int client_socket = *((int *)arg);
  char buffer[BUFFER_SIZE];
  ssize_t bytes_received;

  while (1) {
    // Receive data from the client
    bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) {
      printf("Client disconnected\n");
      break;
    }

    // Null-terminate the received data
    buffer[bytes_received] = '\0';

    // Display the received message
    printf("Received from client: %s", buffer);
  }

  // Close the client socket
  close(client_socket);
  pthread_exit(NULL);
}
