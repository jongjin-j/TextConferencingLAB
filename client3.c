// client.c
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define PORT 12345
#define BUFFER_SIZE 1024

void *receive_thread(void *arg);

int main() {
  int client_socket;
  struct sockaddr_in server_addr;
  pthread_t tid;

  // Create client socket
  if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Configure server address
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  inet_aton(SERVER_IP, &server_addr.sin_addr);

  // Connect to the server
  if (connect(client_socket, (struct sockaddr *)&server_addr,
              sizeof(server_addr)) == -1) {
    perror("Connection failed");
    exit(EXIT_FAILURE);
  }

  printf("Connected to the server.\n");

  // Create a thread to receive messages
  if (pthread_create(&tid, NULL, receive_thread, (void *)&client_socket) != 0) {
    perror("Thread creation failed");
    exit(EXIT_FAILURE);
  }

  // Main loop to send messages
  char message[BUFFER_SIZE];
  while (1) {
    printf("Enter message: ");
    fgets(message, sizeof(message), stdin);

    // Send the message to the server
    send(client_socket, message, strlen(message), 0);
  }

  // Close the client socket
  close(client_socket);
  return 0;
}

void *receive_thread(void *arg) {
  int client_socket = *((int *)arg);
  char buffer[BUFFER_SIZE];
  ssize_t bytes_received;

  while (1) {
    // Receive data from the server
    bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) {
      printf("Server disconnected\n");
      exit(EXIT_SUCCESS);
    }

    // Null-terminate the received data
    buffer[bytes_received] = '\0';

    // Display the received message
    printf("Received from server: %s", buffer);
  }

  pthread_exit(NULL);
}
