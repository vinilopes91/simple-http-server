#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "dqueue.h"

#define BUFFER_SIZE 4096
#define SERVER_PORT 80
#define THREAD_POOL_SIZE 20

void *thread_handler(void *arg);
void connection_handler(int socket_desc);
void send_file(int client, int fd);
void send_headers(int client);
void bad_request(int client);
void file_not_found(int client);

pthread_t thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main()
{
    int server_socket;
    Queue *qu;

    qu = queue_create();

    if (qu == NULL)
    {
        exit(1);
    }

    for (int i = 0; i < THREAD_POOL_SIZE; i++)
    {
        pthread_create(&thread_pool[i], NULL, thread_handler, (void *)qu);
    }

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

    printf("Aguardando conexões\n\n");

    while (1)
    {
        int client_socket;
        client_socket = accept(server_socket, NULL, NULL);

        if (client_socket == -1)
        {
            perror("accept");
            continue;
        }

        pthread_mutex_lock(&mutex);
        queue_push(qu, client_socket);
        pthread_mutex_unlock(&mutex);
    }

    close(server_socket);
    queue_free(qu);

    return 0;
}

void *thread_handler(void *arg)
{
    int client_socket;
    Queue *qu = (Queue *)arg;
    while (1)
    {
        client_socket = 0;
        pthread_mutex_lock(&mutex);
        queue_pop(qu, &client_socket);
        pthread_mutex_unlock(&mutex);

        if (client_socket)
        {
            connection_handler(client_socket);
        }

    }
}

void connection_handler(int socket_desc)
{
    char client_message[BUFFER_SIZE];
    int received_characters = 0;

    received_characters = recv(socket_desc, client_message, BUFFER_SIZE, 0);

    if (received_characters < 1)
    {
        close(socket_desc);
        return;
    }

    printf("%s", client_message);

    char *method, *filename;
    method = strtok(client_message, " ");
    filename = strtok(NULL, " ") + 1;

    if (strcmp(method, "GET") != 0)
    {
        bad_request(socket_desc);
        close(socket_desc);
        return;
    }

    int fp = open(filename, O_RDONLY);

    if (fp < 0)
    {
        file_not_found(socket_desc);
        close(socket_desc);
        return;
    }

    send_headers(socket_desc);
    send_file(socket_desc, fp);

    close(socket_desc);

    return;
}

void send_headers(int client)
{
    char buffer[1024];

    strcpy(buffer, "HTTP/1.1 200 OK\r\n");
    send(client, buffer, strlen(buffer), 0);
    strcpy(buffer, "\r\n");
    send(client, buffer, strlen(buffer), 0);
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
