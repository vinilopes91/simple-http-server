#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#define BUFFER_SIZE 4096
#define SERVER_PORT 80

void *connection_handler(void *);

void send_file(int client, int fd);

void send_headers(int client);

void bad_request(int client);

void file_not_found(int client);

int main()
{
    int server_socket, *new_sock;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == -1)
    {
        perror("socket");
        return 0;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("bind");
        close(server_socket);
        return 0;
    }

    if (listen(server_socket, 5) == -1)
    {
        perror("listen");
        close(server_socket);
        return 0;
    }

    while (1)
    {
        int client_socket;
        client_socket = accept(server_socket, NULL, NULL);

        if (client_socket == -1)
        {
            perror("accept");
            continue;
        }

        pthread_t worker_thread;
		new_sock = malloc(sizeof(int));
		*new_sock = client_socket;

        if( pthread_create(&worker_thread, NULL, connection_handler, (void*) new_sock) < 0)
		{
			perror("could not create thread");
            close(server_socket);
			return 1;
		}
    }
    close(server_socket);

    return 0;
}

void send_file(int client, int fd)
{
    char buffer[1024];
    int bytes_read;

    do
    {
        bzero(buffer, 1024);
        bytes_read = read(fd, buffer, 1024);
        if (bytes_read > 0)
        {
            send(client, buffer, bytes_read, 0);
        }
    } while (bytes_read > 0);

    close(fd);
}

void *connection_handler(void *socket_desc)
{
    int sock = *(int *)socket_desc;
    char client_message[BUFFER_SIZE];
    int received_characters = 0;

    received_characters = recv(sock, client_message, BUFFER_SIZE, 0);

    if (received_characters < 1)
    {
        printf("Fail reading request\n");
        close(sock);
        return NULL;
    }

    printf("%s", client_message);

    char *method, *filename;
    method = strtok(client_message, " ");
    filename = strtok(NULL, " ") + 1;

    if (strcmp(method, "GET") != 0)
    {
        bad_request(sock);
        close(sock);
        return NULL;
    }

    int fp = open(filename, O_RDONLY);

    if (fp < 0)
    {
        file_not_found(sock);
        close(sock);
        return NULL;
    }

    send_headers(sock);
    send_file(sock, fp);

    close(sock);

    return NULL;
}

void send_headers(int client)
{
    char buffer[1024];

    strcpy(buffer, "HTTP/1.1 200 OK\r\n");
    send(client, buffer, strlen(buffer), 0);
    strcpy(buffer, "\r\n");
    send(client, buffer, strlen(buffer), 0);
}

void bad_request(int client)
{
    char buffer[1024];

    sprintf(buffer, "HTTP/1.1 400 BAD REQUEST\r\n");
    send(client, buffer, sizeof(buffer), 0);
}

void file_not_found(int client)
{
    char buffer[1024];

    sprintf(buffer, "HTTP/1.1 404 NOT FOUND\r\n");
    send(client, buffer, strlen(buffer), 0);
}
