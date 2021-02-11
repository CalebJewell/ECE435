#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#include <fcntl.h>

#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE	256

/* Default port to listen on */
#define DEFAULT_PORT	8080
#define  Size 50

int main(int argc, char **argv) {

	int socket_fd,new_socket_fd;
	struct sockaddr_in server_addr, client_addr;
	int port=DEFAULT_PORT;
	int n;
	socklen_t client_len;
	char buffer[BUFFER_SIZE];

	/* My Variables */ 
	char *str;
	char *fileptr;
	char ff[100]; 
	int fd, i = 0;
	
	time_t t ; 
	struct tm *tmp ; 
	char MY_TIME[Size]; 
	time( &t ); 
	      
	//localtime() uses the time pointed by t , 
	// to fill a tm structure with the  
	// values that represent the  
	// corresponding local time. 
      
	tmp = localtime( &t ); 
      
	// using strftime to display time 
	strftime(MY_TIME, sizeof(MY_TIME), "%x - %I:%M%p", tmp);
	printf("Formatted date & time : %s\n", MY_TIME ); 

	/* Open a socket to listen on */
	/* AF_INET means an IPv4 connection */
	/* SOCK_STREAM means reliable two-way connection (TCP) */
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd<0) {
		fprintf(stderr,"Error opening socket! %s\n",
			strerror(errno));
		exit(1);
	}

	/* Set up the server address to listen on */
	/* The memset stes the address to 0.0.0.0 which means */
	/* listen on any interface. */
	memset(&server_addr,0,sizeof(struct sockaddr_in));
	server_addr.sin_family=AF_INET;
	/* Convert the port we want to network byte order */
	server_addr.sin_port=htons(port);

	/* Bind to the port */
	if (bind(socket_fd, (struct sockaddr *) &server_addr,
		sizeof(server_addr)) <0) {
		fprintf(stderr,"Error binding! %s\n", strerror(errno));
		fprintf(stderr,"Probably in time wait, have to wait 60s if you ^C to close\n");
		exit(1);
	}

	/* Tell the server we want to listen on the port */
	/* Second argument is backlog, how many pending connections can */
	/* build up */
	listen(socket_fd,5);

wait_for_connection:

	/* Call accept to create a new file descriptor for an incoming */
	/* connection.  It takes the oldest one off the queue */
	/* We're blocking so it waits here until a connection happens */
	client_len=sizeof(client_addr);
	new_socket_fd = accept(socket_fd,
			(struct sockaddr *)&client_addr,&client_len);
	if (new_socket_fd<0) {
		fprintf(stderr,"Error accepting! %s\n",strerror(errno));
		exit(1);
	}



	while(1) {
		/* Someone connected!  Let's try to read BUFFER_SIZE-1 bytes */
		memset(buffer,0,BUFFER_SIZE);
		n = read(new_socket_fd,buffer,(BUFFER_SIZE-1));

		if (n==0) {
			fprintf(stderr,"Connecion to client lost\n\n");
			break;
		}
		else if (n<0) {
			fprintf(stderr,"Error reading from socket %s\n",
				strerror(errno));
		}
		
		/* Print the message we received */
		printf("Message received: %s\n",buffer);

		str = strstr(buffer,"GET");

		if (str != NULL) {
			
			memset(ff,0,100);
		
			/* skip all characters till the first '/' */
			while(*str != '/') str++;
			str++;

			/* Store the .html file inside another variable */ 
			i = 0;
			while(*str != ' '){
				printf("%c",*str);
				ff[i] = *str;
				i++;
				str++;
			}
	
			ff[i] = '\0';

			printf("******File*******: %s",ff);
			printf("\n");
		}
	
		/* open the file and check for errors */ 
		fd = open(ff,O_RDWR);

		if (fd == '\0') {
			printf("Error opening file! \n");
			break;
		}

		
		/* Send a response */
		n = write(new_socket_fd,"Got your message, thanks!\r\n\r\n",29);
		if (n<0) {
			fprintf(stderr,"Error writing. %s\n",
				strerror(errno));
		}

	}

	close(new_socket_fd);

	printf("Done connection, go back and wait for another\n\n");

	goto wait_for_connection;

	/* Try to avoid TIME_WAIT */
//	sleep(1);

	/* Close the sockets */
	close(socket_fd);

	return 0;
}
