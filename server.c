// source for TCP: https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
// source for multi clients: https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/

// note: a user is someone registered in the database, we have their login credentials
// a client is someone trying to authenticate, ie. a user who is active right now

#include <stdio.h> 
#include <netdb.h> 
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
// #define PORT 8030 
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

// Function designed for chat between client and server. 
void func(int connfd) 
{ 
	char buff[MAX]; 
	int n; 
	// infinite loop for chat 
	for (;;) { 
		bzero(buff, MAX); 

		// read the message from client and copy it in buffer 
		read(connfd, buff, sizeof(buff)); 
		// print buffer which contains the client contents 
		
		//--------------------------------------------------------------------------
		//IMPLEMNETING THE COMMANDS ON THE STDIN FILE STREAM
		// if msg contains "login" then 
		if (strncmp("/login", buff, strlen("/login")) == 0) { 
			//will need to perform some error checking in here
			//for things like correct types, handle cases for missing arguments, etc.
			printf("COMMAND: LOGIN...\n"); 
			char *token = strtok(buff, " "); //assuming space as delimter
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

		if (strncmp("/logout", buff, strlen("/logout")) == 0) { 
			printf("COMMAND: LOGOUT...\n"); 
		} 

		if (strncmp("/joinsession", buff, strlen("/joinsession")) == 0) { 
			printf("COMMAND: JOIN_SESSION...\n"); 
			char *token = strtok(buff, " "); //assuming space as delimter
			token = strtok(NULL, " "); //get second argument
			printf("session_ID is: %s\n", token);
			char* session_ID = token;
		} 

		if (strncmp("/leavesession", buff, strlen("/leavesession")) == 0) { 
			printf("COMMAND: LEAVE_SESSION...\n"); 
		} 

		if (strncmp("/createsession", buff, strlen("/createsession")) == 0) { 
			printf("COMMAND: CREATE_SESSION...\n"); 
			char *token = strtok(buff, " "); //assuming space as delimter
			token = strtok(NULL, " "); //get second argument
			printf("session_ID is: %s\n", token);
			char* session_ID = token;

			sessionArray = (struct Session *)malloc((numberOfSessions+1) * sizeof(struct Session));
			sessionArray[numberOfSessions].session_ID = atoi(session_ID);	//no sessions created yet 
			// so create this session
			// and then update the number of clients in the session
			sessionArray[numberOfSessions].numClientsInSession = 1;

			// will need to get the client_ID here...
			// strcpy(sessionArray[numberOfSessions].clients[0], client_ID);

			numberOfSessions++;			
		} 

		if (strncmp("/list", buff, strlen("/list")) == 0) { 
			printf("COMMAND: LIST...\n"); 
		} 

		printf("From client: %s\t To client : ", buff); 
		bzero(buff, MAX); 
		n = 0; 
		// copy server message in the buffer 
		while ((buff[n++] = getchar()) != '\n') 
			; 

		// and send that buffer to client 
		write(connfd, buff, sizeof(buff)); 

		// if msg contains "Exit" then server exit and chat ended. 
		if (strncmp("quit", buff, 4) == 0) { 
			printf("Server Quit...\n"); 
			break; 
		} 
	} 
} 

// Driver function 
int main(int argc, char *argv[]) 
{ 	
	int client_socket[MAX_CLIENTS];
	//initialize all client_socket to 0
	for(int i = 0; i < MAX_CLIENTS; i++){
		client_socket[i] = 0;
	}

	int sockfd, connfd, len; 
	struct sockaddr_in servaddr, cli; 

	int portNumber = atoi(argv[1]);
	unsigned short int PORT = (short)portNumber;


	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(PORT); 

	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully binded..\n"); 

	// Now server is ready to listen and verification 
	if ((listen(sockfd, 5)) != 0) { 
		printf("Listen failed...\n"); 
		exit(0); 
	} 
	else
		printf("Server listening..\n"); 
	len = sizeof(cli); 

	// Accept the data packet from client and verification 
	connfd = accept(sockfd, (SA*)&cli, &len); 
	if (connfd < 0) { 
		printf("server accept failed...\n"); 
		exit(0); 
	} 
	else
		printf("server accept the client...\n"); 

	// Function for chatting between client and server 
	func(connfd); 

	// After chatting close the socket 
	close(sockfd); 
}
