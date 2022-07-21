#include "header.h"

int isValid( SOCKET i, char* str)
{
	if (i < 0) {
		fprintf(stderr, "%s failed. (%d)\n", str, GETSOCKETERRNO());
        	return 0;
	}
	return 1;
}

int main() {

    printf("Configuring local address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bind_address;
    getaddrinfo(0, "8080", &hints, &bind_address);

    printf("Creating socket...\n");
    SOCKET socket_listen;
    socket_listen = socket(bind_address->ai_family,
            bind_address->ai_socktype, bind_address->ai_protocol);
    isValid(socket_listen, "socket()");


    printf("Binding socket to local address...\n");
    isValid(bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen), "bind()");
    freeaddrinfo(bind_address);


    printf("Listening...\n");
    isValid(listen(socket_listen, 10), "listen()");

    // Making the file descriptor set!
    fd_set master;
    FD_ZERO(&master);
    FD_SET(socket_listen, &master);
    SOCKET max_socket = socket_listen;

    printf("Waiting for connections...\n");

    while(1) {
        fd_set reads;
        reads = master;
        isValid(select(max_socket+1, &reads, 0, 0, 0), "select()");
        
        // Select is a function that sees which sockets are ready to be read such that we can distribute data
        //I will check which of these sockets is set!
        SOCKET i;
        for (i = 1; i <= max_socket; i++)
        {
        	if (FD_ISSET(i, &reads)) {
        		// New Connection to handle
        		if (i == socket_listen)
        		{
        			struct sockaddr_in address;
    				socklen_t addrlen = sizeof(address);
        			SOCKET socket_client = accept(socket_listen, (struct sockaddr*)&address, &addrlen);
        			isValid(socket_client, "accept()");
        			
        			// Added the client to the set
        			FD_SET(socket_client, &master);
        			if (socket_client > max_socket) {
                        		max_socket = socket_client;
                        	}
        			printf("[+] New connnection \n");
        			
        		// Connection already exists... Send the message recv to all other clients except for the one it matches too or the socket_listen
        		} else {
        			char buffer[1024];
        			int total_bytes = recv(i, &buffer, 1024, 0);
        			if (total_bytes < 1)
        			{
        				FD_CLR(i, &master);
        				CLOSESOCKET(i);
        				continue;
        			}
        			printf("[+] (%d) bytes recieved\n", total_bytes);
        			
        			for (int j = 1; j <= max_socket; j++)
        			{
        				if (FD_ISSET(j, &master)) {
        				
        					if (j == socket_listen || j == i)
        					{
        						continue;
        					} else {
        						send(j, buffer, total_bytes, 0);
        					}
        				}
        			}
        			
        			
        		}
        	
        	}
        }
       

    }


    printf("Finished.\n");

    return 0;
}
