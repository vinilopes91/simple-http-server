#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 4096

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
    close(client);
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

    close(client);
}

void file_not_found(int client)
{
    char buffer[1024];

    sprintf(buffer, "HTTP/1.1 404 NOT FOUND\r\n");
    send(client, buffer, strlen(buffer), 0);

    close(client);
}

int main()
{
    int server_socket;
    int received_characters = 0;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == -1)
    {
        perror("socket");
        return 0;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(80);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("bind");
        return 0;
    }

    if (listen(server_socket, 4096) == -1)
    {
        perror("listen");
        return 0;
    }

    while (1)
    {
        int client_socket;
        char client_message[BUFFER_SIZE];
        client_socket = accept(server_socket, NULL, NULL);

        if (client_socket == -1)
        {
            perror("accept");
            continue;
        }

        received_characters = recv(client_socket, client_message, BUFFER_SIZE, 0);

        if (received_characters < 1)
        {
            printf("Fail reading request\n");
            continue;
        }

        printf("%s", client_message);

        char *method, *filename;
        method = strtok(client_message, " ");
        filename = strtok(NULL, " ") + 1;

        if (strcmp(method, "GET") != 0)
        {
            bad_request(client_socket);
            continue;
        }

        int fp = open(filename, O_RDONLY);

        if (fp < 0)
        {
            file_not_found(client_socket);
            continue;
        }

        send_headers(client_socket);
        send_file(client_socket, fp);
    }
    close(server_socket);

    return 0;
}
