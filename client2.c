#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

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
