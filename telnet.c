#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#define WILL 0xfb
#define WONT 0xfc
#define DO 0xfd
#define DONT 0xfe
#define IAC 0xff

int main(int argc, char const *argv[])
{	
	if(argc != 2 && argc != 3){
		printf("Incorrect format.\nman telnet for more information.\n");
		exit(1);
	}
	// assign default port number to port_no.
	int port_no = 23;
	// change port_no if specified.
	if(argc == 3){
		port_no = atoi(argv[2]);
	}
	// struct sockaddr_in ip_address;
	struct sockaddr_in server_address;
	int socket_no = 0;
	char buffer[1024] = {0};
	char *line = NULL;
	size_t len = 0;

	// create socket
	socket_no = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_no <= 0){
		printf("Socket creation failed!\n");
		exit(1);
	}

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port_no);

	int errno = inet_pton(AF_INET, argv[1], &server_address.sin_addr);
	if(errno <= 0){
		// if the ip address is invalid the string entered might be domain name.
		// converting domain name to ip.
		struct hostent *ip_convert = gethostbyname(argv[1]);
		if(ip_convert == NULL){
			printf("Invalid Domain Name / IP Address !\n");
			exit(1);
		}
		char addr[32];
		struct in_addr **addr_list = (struct in_addr **) ip_convert->h_addr_list;
		if(addr_list[0] != NULL){
			strcpy(addr,inet_ntoa(*addr_list[0]));
		}
		errno = inet_pton(AF_INET, addr, &server_address.sin_addr);
		if(errno <= 0){
			printf("Invalid Domain Name / IP Address !\n");
			exit(1);
		}
	}

	struct sockaddr *addr_socket = (struct sockaddr *)&server_address;
	socklen_t addrlen = sizeof(server_address);
	printf("Trying to connect to IP : %s \n",inet_ntoa(server_address.sin_addr));
	if(connect(socket_no, addr_socket, addrlen) < 0){
		printf("Connection refused!\n");
		exit(1);
	}

	printf("Connected to IP : %s \n",inet_ntoa(server_address.sin_addr));

	fd_set readfds;
	int return_value;
	int input_length;
	while(true){
		FD_ZERO(&readfds);
		FD_SET(socket_no, &readfds);
		FD_SET(0, &readfds);

		int socket_op = socket_no + 1;

		return_value = select(socket_op, &readfds, NULL, NULL, NULL);

		if(return_value < 0){
			printf("select() returned an error.\n");
			exit(1);
		}

		if( !(socket_no == 0) && FD_ISSET(socket_no, &readfds)){
			//data is there in the socket_no to be read and displayed.
			// printf("here1\n");
			int e = read(socket_no, buffer, 1);
			if(e == -1){
				printf("Connection closed by the server (no data sent by the server).\n");
				exit(1);
			}
			if(e == 0){
				printf("Connection closed by the server (no data sent by the server).\n");
				exit(1);
			}
			if(buffer[0] == IAC ){
				input_length = read(socket_no, buffer + 1, 2);
				if( input_length == -1){
					printf("read() returned an error.\n");
					exit(1);
				}
				if( input_length == 0){
					printf("Connection closed by the server (no data sent by the server).\n");
					exit(1);
				}

				if(buffer[1] == DO){
					buffer[1] = WONT;
				}
				if(buffer[1] == WILL){
					buffer[1] = DO;
				}
				if(buffer[2] == DO){
					buffer[2] = WONT;
				}
				if(buffer[2] == WILL){
					buffer[2] = DO;
				}				
				int er = send(socket_no, buffer, 3, 0);
				if(er == -1){
					printf("send() returned an error\n");
					exit(1);
				}
			}
			else{
				buffer[1] = '\0';
				printf("%s", buffer);
				fflush(0);
			}
		}
		if(FD_ISSET(0, &readfds)){
			int length_read = getline(&line, &len, stdin);
			if(length_read == -1){
				buffer[0] = '\n';
				int er = send(socket_no, buffer, 1, 0);
				if(er == -1){
					printf("send() returned an error.\n");
					exit(1);
				}
			}
			else{
				line[length_read - 1] = '\r';
				line[length_read] = '\n';
				int er = send(socket_no, line, length_read+1, 0);
				if(er == -1){
					printf("send() returned an error.\n");
					exit(1);
				}
			}
		}
	}
    close(socket_no);
	return 0;
}