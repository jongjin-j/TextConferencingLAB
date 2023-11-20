// source for TCP: https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
// source for multi clients: https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/

// note: a user is someone registered in the database, we have their login credentials
// a client is someone trying to authenticate, ie. a user who is active right now

#include <stdio.h> 
#include <stdbool.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> // read(), write(), close()
#define MAX 80 

#define MAX_USERS 3
#define MAX_USERNAME_LENGTH 80
#define MAX_PASSWORD_LENGTH 80
#define PORT 8030 
#define SA struct sockaddr 

//-----------SESSION DATA STRUCTURES------/
#define MAX_CLIENTS 3
int numberOfSessions = 0;
struct Session{
	int session_ID;
	int clients[MAX_CLIENTS];
	int numClientsInSession;
};
struct Session *sessionArray = NULL;

//-----------CLIENT DATA STRUCTURES------/
#define MAX_CLIENTS 3
int numberOfClients = 0;
struct Client{
	char client_ID [MAX];
	int session_ID;
	int ip_addr;
	int port_addr;
};
struct Client *clientArray = NULL;

//-----------USER DATA STRUCTURES------/
struct User{
	char username[MAX_USERNAME_LENGTH];
	char password[MAX_USERNAME_LENGTH];
};

//-----------HARDCODED USER DATA------/
struct User users[MAX_USERS] = {
		{"jill", "eW94dsol"},
		{"jack", "432wlFd"},
		{"user3", "pass3"},
};

// // Function designed for chat between client and server. 
// void func(int connfd, char buff[1024]) 
// { 
// 	// char buff[MAX]; 
// 	int n; 
// 	// infinite loop for chat 
// 	for (;;) { 
// 		bzero(buff, MAX); 

// 		// read the message from client and copy it in buffer 
// 		read(connfd, buff, sizeof(buff)); 
// 		// print buffer which contains the client contents 
		
// 		//--------------------------------------------------------------------------
// 		//IMPLEMNETING THE COMMANDS ON THE STDIN FILE STREAM
// 		// if msg contains "login" then 
// 		if (strncmp("/login", buff, strlen("/login")) == 0) { 
// 			//will need to perform some error checking in here
// 			//for things like correct types, handle cases for missing arguments, etc.
// 			printf("COMMAND: LOGIN...\n"); 
// 			char *token = strtok(buff, " "); //assuming space as delimter
// 			token = strtok(NULL, " "); //get second argument
// 			printf("client_ID is: %s\n", token);
// 			char* client_ID = token;

// 			token = strtok(NULL, " "); //get third argument
// 			printf("password is: %s\n", token);
// 			char *password = token;

// 			token = strtok(NULL, " "); //get fourth argument
// 			printf("ip addr is: %s\n", token);
// 			char *addr = token;

// 			token = strtok(NULL, " "); //get fifth argument
// 			printf("port is: %s\n", token);
// 			char *port = token;

// 			//-------------authenticate user
// 			int authenticate = 0;
// 			for(int i = 0; i<MAX_USERS; i++){
// 				if(strcmp(users[i].username, client_ID)==0){
// 					if(strcmp(users[i].password, password)==0){
// 						printf("Authentication successful.\n");

// 						clientArray = (struct Client *)malloc((numberOfClients+1) * sizeof(struct Client));
// 						strcpy(clientArray[numberOfClients].client_ID, client_ID);
// 						clientArray[numberOfClients].session_ID = -1;	//no sessions created yet 
// 						numberOfClients++;

// 						authenticate = 1;
// 					}
// 				}
// 			}
// 			if(authenticate == 0){
// 				printf("Authentication failed!\n");
// 			}

// 			for(int i = 0; i<numberOfClients; i++){
// 				printf("%s\n", clientArray[i].client_ID);
// 			}
// 		} 

// 		if (strncmp("/logout", buff, strlen("/logout")) == 0) { 
// 			printf("COMMAND: LOGOUT...\n"); 
// 		} 

// 		if (strncmp("/joinsession", buff, strlen("/joinsession")) == 0) { 
// 			printf("COMMAND: JOIN_SESSION...\n"); 
// 			char *token = strtok(buff, " "); //assuming space as delimter
// 			token = strtok(NULL, " "); //get second argument
// 			printf("session_ID is: %s\n", token);
// 			char* session_ID = token;
// 		} 

// 		if (strncmp("/leavesession", buff, strlen("/leavesession")) == 0) { 
// 			printf("COMMAND: LEAVE_SESSION...\n"); 
// 		} 

// 		if (strncmp("/createsession", buff, strlen("/createsession")) == 0) { 
// 			printf("COMMAND: CREATE_SESSION...\n"); 
// 			char *token = strtok(buff, " "); //assuming space as delimter
// 			token = strtok(NULL, " "); //get second argument
// 			printf("session_ID is: %s\n", token);
// 			char* session_ID = token;

// 			sessionArray = (struct Session *)malloc((numberOfSessions+1) * sizeof(struct Session));
// 			sessionArray[numberOfSessions].session_ID = atoi(session_ID);	//no sessions created yet 
// 			// so create this session
// 			// and then update the number of clients in the session
// 			sessionArray[numberOfSessions].numClientsInSession = 1;

// 			// will need to get the client_ID here...
// 			// strcpy(sessionArray[numberOfSessions].clients[0], client_ID);

// 			numberOfSessions++;			
// 		} 

// 		if (strncmp("/list", buff, strlen("/list")) == 0) { 
// 			printf("COMMAND: LIST...\n"); 
// 		} 

// 		printf("From client: %s\t To client : ", buff); 
// 		bzero(buff, MAX); 
// 		n = 0; 
// 		// copy server message in the buffer 
// 		while ((buff[n++] = getchar()) != '\n') 
// 			; 

// 		// and send that buffer to client 
// 		write(connfd, buff, sizeof(buff)); 

// 		// if msg contains "Exit" then server exit and chat ended. 
// 		if (strncmp("quit", buff, 4) == 0) { 
// 			printf("Server Quit...\n"); 
// 			break; 
// 		} 
// 	} 
// } 

// Driver function 
int main(int argc, char *argv[]) 
{ 	
	// new variables
	int opt = true;
	int master_socket, addrlen, new_socket, client_socket[30], max_clients = 30, activity, i, valread, sd;

	int max_sd;
	struct sockaddr_in address;

	char buffer[1025]; //data buffer of 1k
	
	//set of socket descriptrs
	fd_set readfds;

	// a message
	char *message = "msg from server to me.\n";

	// int client_socket[MAX_CLIENTS];
	//initialize all client_socket to 0
	for(int i = 0; i < MAX_CLIENTS; i++){
		client_socket[i] = 0;
	}

	int sockfd, connfd, len; 
	// struct sockaddr_in servaddr, cli; 

	// int portNumber = atoi(argv[1]);
	// unsigned short int PORT = (short)portNumber;

	// create a master socket
	if((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0){
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	//set master socket to allow multiple connections
	// good habit, program would still work without this tho
	if(setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0){
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}


	// // socket create and verification 
	// sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	// if (sockfd == -1) { 
	// 	printf("socket creation failed...\n"); 
	// 	exit(0); 
	// } 
	// else
	// 	printf("Socket successfully created..\n"); 
	// bzero(&servaddr, sizeof(servaddr)); 

	// types of socket created
	// assign IP, PORT 
	address.sin_family = AF_INET; 
	// servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons(PORT); 

	// bind the socket to localhost port 8030
	if(bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0){
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	printf("Listener on port %d \n", PORT);

	//try to specify maximum of 3 pending connections for the master socket
	if(listen(master_socket, 3) < 0){
		perror("listen");
		exit(EXIT_FAILURE);
	}

	//accept the incoming connection
	addrlen = sizeof(address);
	puts("Waiting for connections...");

	while(true){
		//clear the socket set
		FD_ZERO(&readfds);

		//add master socket to the set
		FD_SET(master_socket, &readfds);
		max_sd = master_socket;

		// add child socket to set
		for(i = 0; i<max_clients; i++){
			// socket descriptor
			sd = client_socket[i];
			// if valid socket descriptor then add to read list
			if(sd > 0){
				FD_SET(sd, &readfds);
			}
			//highest file descriptor number, need it for the select function
			if(sd > max_sd){
				max_sd = sd;
			}
		}
		//wait for an activity on one of the sockets, timeour is NULL, so wait indefinitely
		activity = select(max_sd +1, &readfds, NULL, NULL, NULL);

		if((activity<0) && (errno!=EINTR)){
			printf("Select error!");
		}

		// if something happened on the master socket, then its an incoming connection
		if(FD_ISSET(master_socket, &readfds)){
			if((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0){
				perror("accept");
				exit(EXIT_FAILURE);
			}
		

		//inform user of socket number - used in send and receive commands
		printf("New connection, socket fd is %d, ip is %s, port %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs (address.sin_port));

		//send new connection greeting message
		if(send(new_socket, message, strlen(message), 0) != strlen(message)){
			perror("Send");
		}

		puts("Welcome message sent successfully");

		//add new socket to array of sockets
		for(int i = 0; i<max_clients; i++){
			//if position is empty
			if(client_socket[i] == 0){
				client_socket[i] = new_socket;
				printf("Adding to list of sockets as %d\n", i);
				break;
			}
		}
	}

	//else it has some I/O operation on some other socket
	for(i = 0; i<max_clients; i++){
		sd = client_socket[i];

		if(FD_ISSET(sd, &readfds)){
			//check if it was for closing, and also read the incoming message
			if((valread = read(sd, buffer, 1024)) == 0){
				//somebody disconnected, get details and print
				getpeername(sd, (struct sockaddr*)&address, \
				 (socklen_t*)&addrlen);
				printf("Host disconnected, ip %s, port %d \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

				//close the socket and mark as 0 in list for reuse
				close(sd);
				client_socket[i] = 0;
			}
		//echo back message that came in
		else{
			buffer[valread] = '\0';
			printf("Message from client: %s", buffer);


			if (strncmp("/login", buffer, strlen("/login")) == 0) { 
			//will need to perform some error checking in here
			//for things like correct types, handle cases for missing arguments, etc.
			printf("COMMAND: LOGIN...\n"); 
			char *token = strtok(buffer, " "); //assuming space as delimter
			token = strtok(NULL, " "); //get second argument
			printf("client_ID is: %s\n", token);
			char* client_ID = token;

			token = strtok(NULL, " "); //get third argument
			printf("password is: %s\n", token);
			char *password = token;

			token = strtok(NULL, " "); //get fourth argument
			printf("ip addr is: %s\n", token);
			char *addr = token;

			token = strtok(NULL, " "); //get fifth argument
			printf("port is: %s\n", token);
			char *port = token;

			//-------------authenticate user
			int authenticate = 0;
			for(int i = 0; i<MAX_USERS; i++){
				if(strcmp(users[i].username, client_ID)==0){
					if(strcmp(users[i].password, password)==0){
						printf("Authentication successful.\n");

						clientArray = (struct Client *)malloc((numberOfClients+1) * sizeof(struct Client));
						strcpy(clientArray[numberOfClients].client_ID, client_ID);
						clientArray[numberOfClients].session_ID = -1;	//no sessions created yet 
						numberOfClients++;

						authenticate = 1;
					}
				}
			}
			if(authenticate == 0){
				printf("Authentication failed!\n");
			}

			for(int i = 0; i<numberOfClients; i++){
				printf("%s\n", clientArray[i].client_ID);
			}
		} 



			send(sd, message, strlen(message), 0);
			break;

		}
	}
}
	}
return 0;
}

// 	// Binding newly created socket to given IP and verification 
// 	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
// 		printf("socket bind failed...\n"); 
// 		exit(0); 
// 	} 
// 	else
// 		printf("Socket successfully binded..\n"); 

// 	// Now server is ready to listen and verification 
// 	if ((listen(sockfd, 5)) != 0) { 
// 		printf("Listen failed...\n"); 
// 		exit(0); 
// 	} 
// 	else
// 		printf("Server listening..\n"); 
// 	len = sizeof(cli); 

// 	// Accept the data packet from client and verification 
// 	connfd = accept(sockfd, (SA*)&cli, &len); 
// 	if (connfd < 0) { 
// 		printf("server accept failed...\n"); 
// 		exit(0); 
// 	} 
// 	else
// 		printf("server accept the client...\n"); 

// 	// Function for chatting between client and server 
// 	func(connfd); 

// 	// After chatting close the socket 
// 	close(sockfd); 
// }
