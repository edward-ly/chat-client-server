// File server.c
// Author: Edward Ly
// Last Modified: 15 December 2016
//
// TCP chat client.
// Usage: ./client [hostname] [port]
// Really only useful with chat server.
// Based on code from Linux HowTos.

#include "c-s-socket.h"

void* read_loop(void* argument) {
    int n, sockfd = *( (int*)argument );
    char buffer[256];

    while (true) {
        bzero(buffer, 256);
        n = read(sockfd, buffer, 255);
        if (n < 0) error("ERROR reading from socket");
        fprintf(stdout, "%s", buffer);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s [hostname] [port]\n", argv[0]);
        exit(1);
    }

    portno = atoi(argv[2]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    server = gethostbyname(argv[1]);
    if (server == NULL) error("ERROR no such host");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) error("ERROR connecting");

    pthread_t read_thread[1];
    int result_code;
    result_code = pthread_create(read_thread, NULL, read_loop, &sockfd);
    if (result_code) error("ERROR creating thread");

    fprintf(stdout, "220 chat client version 0.1 ready.\n");
    fprintf(stdout, "Type \"/help\" to see all available commands.\n");

    char buffer[256], tmp_buffer[256], command[7] = "";

    // write loop
    while (strcmp(command, "/quit") != 0) {
        bzero(buffer, 256);
        fgets(buffer, 255, stdin);

        // get and check command if empty or "/quit"
        strcpy(tmp_buffer, buffer); // so that buffer is not overwritten
        char* tmp = strtok(tmp_buffer, " \n\r\0");
        if (tmp) strncpy(command, tmp, 7);
        else continue; // do nothing if empty
        if (strchr(command, '\0') == NULL) command[6] = '\0';

        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) error("ERROR writing to socket");
    }

    close(sockfd);
    return 0;
}
