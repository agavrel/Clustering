/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
  // inspired from http://www.linuxhowtos.org/C_C++/socket.htm tutorial
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h> // adding thread

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

typedef struct	s_client_data
{
	int					newsockfd;
	struct sockaddr_in	serv_addr;
}				t_client_data;

void *connect_to_server(void *client_data)
{
	t_client_data		*data;
	int					n;
	char				buffer[256];

	data = (t_client_data *)client_data;
	while (42)
	{
		// 5) Send and receive data
		bzero(buffer, 256);
		if ((n = read(data->newsockfd, buffer, 255)) < 0)
			error("ERROR reading from soc0ket");
		printf("From %d: %s\n", data->serv_addr.sin_port, buffer);
		if (!strcmp(buffer, "exit\n"))
		{
			close(data->newsockfd);
			break ;
		}
		if ((n = write(data->newsockfd, buffer, strlen(buffer)) < 0))
			error("ERROR writing to socket");
	}
	pthread_exit(NULL); // ferme le thread
}

static void init_server_data(struct sockaddr_in	*serv_addr, int portno, int *sockfd)
{
	// Get serveur addresss informations
	bzero((char *) serv_addr, sizeof(* serv_addr));
	serv_addr->sin_family = AF_INET;
	serv_addr->sin_addr.s_addr = INADDR_ANY;
	serv_addr->sin_port = htons(portno);
	// 2) Bind the socket to an address using the bind() system call. For a server socket on the Internet, an address consists of a port number on the host machine.
	if (bind(*sockfd, (struct sockaddr *) serv_addr, sizeof(* serv_addr)) < 0)
		error("ERROR on binding");
	// 3) Listen for connections with the listen() system call
	listen(*sockfd, 5); // 5 client max waiting to connect
}

int main(int argc, char *argv[])
{
	int 				sockfd;
	struct sockaddr_in 	cli_addr;
	pthread_t			thr;
	t_client_data		client_data;
	socklen_t 			clilen;

     if (argc < 2)
	 {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
// 1) Create a socket for the client with the socket() system call
     if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // should be recreated if client disconnect
        error("ERROR opening socket");
	init_server_data(&client_data.serv_addr, atoi(argv[1]), &sockfd);
	clilen = sizeof(cli_addr);
	while (42)
	{
	// 4) Accept a connection with the accept() system call. This call typically blocks until a client connects with the server.
		if ((client_data.newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) < 0) // create the client fd and should be in the loop
			 error("ERROR on accept");
	// steps 4) and 5)
		pthread_create(&thr, NULL, connect_to_server, &client_data);
		pthread_join(thr, NULL);
	}
	close(sockfd);
    return (0);
}
