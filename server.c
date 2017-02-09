// File server.c
// Author: Edward Ly
// Last Modified: 15 December 2016
//
// TCP chat server.
// Usage: ./server [port]
// Really only useful with chat client.
// Based on code from Linux HowTos.

#include "c-s-socket.h"

#define MAX_CLIENTS  16
#define MAX_USER_LEN 64
#define MAX_MSG_LEN  192

typedef struct client_t {
    int id;
    char username[MAX_USER_LEN];
    bool isConnected;
    bool isLoggedIn;
} client_t;

pthread_t threads[MAX_CLIENTS];
int thread_args[MAX_CLIENTS];
client_t clients[MAX_CLIENTS];

void* client_loop(void* arg) {
    char buffer[256];
    int n, i = *( (int*)arg );

    // read/write loop
    while (true) {
        bzero(buffer, 256);
        n = read(clients[i].id, buffer, 255);
        if (n < 0) error("ERROR reading from socket");

        fprintf(stderr, "Command received from client %d: %s", clients[i].id, buffer);

        // get command token
        char* command = strtok(buffer, " \n\r\0");
        if (command == NULL) continue; // empty command

        // do something depending on command
        if (strcmp(command, "/user") == 0) { // login user
            char* tmp = strtok(NULL, " \n\r\0");
            if ( tmp && (strlen(tmp) > 0) && (strlen(tmp) < MAX_USER_LEN) && isalpha(tmp[0]) ) {
                strcpy(clients[i].username, tmp);
                fprintf(stderr, "Logging in user %s...\n", clients[i].username);
                char* res = (char*)malloc(sizeof(char) * (MAX_USER_LEN + 14));
                strcpy(res, "250 welcome ");
                strcat(res, clients[i].username);
                strcat(res, "\n");
                n = write(clients[i].id, res, strlen(res));
                if (n < 0) error("ERROR writing to socket");
                clients[i].isLoggedIn = true;
                free(res);
            }
            else { // invalid username
                fprintf(stderr, "Invalid username %s.\n", tmp);
                char* response =  "400 invalid username\n";
                n = write(clients[i].id, response, strlen(response));
                if (n < 0) error("ERROR writing to socket");
                clients[i].isLoggedIn = false;
            }
        }
        else if (strcmp(command, "/post") == 0) { // post message
            if (clients[i].isLoggedIn) {
                char* tmp = strtok(NULL, "\n\r\0");
                if ( tmp && (strlen(tmp) < MAX_MSG_LEN) ) {
                    char message[MAX_MSG_LEN];
                    strcpy(message, tmp);
                    fprintf(stderr, "Message received, sending message...\n");
                    char* response = "221 post command successful\n";
                    n = write(clients[i].id, response, strlen(response));
                    if (n < 0) error("ERROR writing to socket");

                    // build message
                    char* res = (char*)malloc(sizeof(char) * (MAX_USER_LEN + MAX_MSG_LEN + 8));
                    strcpy(res, "210 ");
                    strcat(res, clients[i].username);
                    strcat(res, ": ");
                    strcat(res, message);
                    strcat(res, "\n");

                    // write message to all users
                    int j;
                    for (j = 0; j < MAX_CLIENTS; j++) {
                        if (clients[j].isLoggedIn) {
                            n = write(clients[j].id, res, strlen(res));
                            if (n < 0) error("ERROR writing to socket");
                        }
                    }
                    free(res);
                }
                else { // message too long or too short
                    fprintf(stderr, "Message is invalid.\n");
                    char* response = "400 invalid message\n";
                    n = write(clients[i].id, response, strlen(response));
                    if (n < 0) error("ERROR writing to socket");
                }
            }
            else { // not logged in
                fprintf(stderr, "Client is not logged in.\n");
                char* response = "401 unauthorized\n";
                n = write(clients[i].id, response, strlen(response));
                if (n < 0) error("ERROR writing to socket");
            }
        }
        else if (strcmp(command, "/who") == 0) { // send list of users
            if (clients[i].isLoggedIn) {
                fprintf(stderr, "Sending list of users to client %d...\n", clients[i].id);

                // send list one entry at a time
                char* res = (char*)malloc(sizeof(char) * (MAX_USER_LEN + 6)); int j;
                for (j = 0; j < MAX_CLIENTS; j++) {
                    if (clients[j].isLoggedIn) {
                        strcpy(res, "211-");
                        strcat(res, clients[j].username);
                        strcat(res, "\n");

                        n = write(clients[i].id, res, strlen(res));
                        if (n < 0) error("ERROR writing to socket");
                    }
                }
                free(res);

                char* response = "211 end of user list\n";
                n = write(clients[i].id, response, strlen(response));
                if (n < 0) error("ERROR writing to socket");
            }
            else { // not logged in
                fprintf(stderr, "Client is not logged in.\n");
                char* response = "401 unauthorized\n";
                n = write(clients[i].id, response, strlen(response));
                if (n < 0) error("ERROR writing to socket");
            }
        }
        else if (strcmp(command, "/quit") == 0) { // disconnect from client
            fprintf(stderr, "Disconnecting from client %d...\n", clients[i].id);
            char* response = "222 disconnect\n";
            n = write(clients[i].id, response, strlen(response));
            if (n < 0) error("ERROR writing to socket");
            break;
        }
        else { // "/help" or unrecognized command
            if (strcmp(command, "/help") == 0)
                fprintf(stderr, "Help received, sending usage to client...\n");
            else fprintf(stderr, "Unknown command %s, sending usage to client...\n", command);

            char* response = "Available commands:\n/user [name] - login with the given name\n/post [message] - post message to all users\n/who - get list of logged in users\n/help - display this help message\n/quit - disconnect from server\n";
            n = write(clients[i].id, response, strlen(response));
            if (n < 0) error("ERROR writing to socket");
        }
    }

    clients[i].isLoggedIn = false;
    clients[i].isConnected = false;
    close(clients[i].id);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(1);
    }

    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    // initialize client settings
    for (n = 0; n < MAX_CLIENTS; n++) {
        clients[n].isConnected = false;
        clients[n].isLoggedIn = false;
        thread_args[n] = n;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) error("ERROR on binding");

    fprintf(stderr, "Chat server version 0.1\n");
    fprintf(stderr, "Waiting for connection...\n");

    int result_code, i;

    // listen for new clients requesting a connection
    while (true) {
        listen(sockfd, 5);
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) error("ERROR on accept");

        i = 0;
        while (clients[i].isConnected) i++;
        if (i >= MAX_CLIENTS) {
            char* response = "503 room is full, please try again later\n";
            n = write(newsockfd, response, strlen(response));
            if (n < 0) error("ERROR writing to socket");
            close(newsockfd);
            continue;
        }

        // create a new thread for client
        clients[i].id = newsockfd;
        clients[i].isConnected = true;
        clients[i].isLoggedIn = false;
        result_code = pthread_create(threads + i, NULL, client_loop, thread_args + i);
        if (result_code) error("ERROR creating thread");

        fprintf(stderr, "Client connected with id %d.\n", newsockfd);
    }

    close(sockfd);
    return 0;
}
