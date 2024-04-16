#include <stdio.h>
#include <time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()
#define MAX 80
#define PORT 8080
#define SA struct sockaddr

// Function for the chat between the client and the server
void func(int sockfd)
{
    char buff[MAX];
    int n;
    // Loop for the chat
    for (;;)
    {
        bzero(buff, MAX);

        // Read the message from client and copy it to buffer
        read(sockfd, buff, sizeof(buff));

        // Display the buffer which contains the client contents
        printf("Del cliente: %s\n Al cliente: ", buff);
        bzero(buff, MAX);
        n = 0;
        // Copy server message to the buffer
        while ((buff[n++] = getchar()) != '\n')
            ;

        // and send that buffer to client
        write(sockfd, buff, sizeof(buff));

        // if msg contains "exit" then server exit and chat ended.
        if (strncmp("SALIR", buff, 4) == 0)
        {
            printf("Saliendo...\n");
            break;
        }
    }
}

// Main function
int main()
{
    int sockfd, connfd, len;
    pid_t childpid;
    struct sockaddr_in servaddr, cli;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("Falla creacion de socket...\n");
        exit(0);
    }
    else
        printf("Socket creado...\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Bind newly created socket to given IP and verify
    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("Socket bind falló...\n");
        exit(0);
    }
    else
        printf("Socket bind exitoso..\n");

    // Server is now ready to listen and verify
    if ((listen(sockfd, 5)) != 0)
    {
        printf("Listen falla...\n");
        exit(0);
    }
    else
        printf("Server escuchando...\n");
    len = sizeof(cli);

    // Accept the data packet from client and verify
    while (1)
    {
        connfd = accept(sockfd, (SA *)&cli, &len);
        if (connfd < 0)
        {
            printf("Server accept falla...\n");
            exit(0);
        }
        else
            printf("\nServer accepta el cliente...\n");

        // Crea child
        childpid = fork();
        if (childpid == 0)
        {
            // If child, cierro server socket
            close(sockfd);

            // Chat
            func(connfd);

            // Terminado el chat cierro el socket del cliente
            close(connfd);
            break; // Salgo del child despues de terminar el chat
        }
        else {
            // Si soy el padre, cierro socket del cliente y continuo escuchando
            close(connfd);
        }
    }
    // cierro el server socket
    close(sockfd);
}
