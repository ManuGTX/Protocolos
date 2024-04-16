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

// calculador de factorial
int factorial(int n) {
    if (n >= 1) {
        return n * factorial(n - 1);
    }
    else {
        return 1;
    }
}

// funcion calculadora de tiempo
double tiempo_transcurrido(struct timespec *inicio, struct timespec *fin) {
    double start_sec = (double)inicio->tv_sec + (double)inicio->tv_nsec / 1e9;
    double end_sec = (double)fin->tv_sec + (double)fin->tv_nsec / 1e9;
    return end_sec - start_sec;
}

// Function for the chat between the client and the server
void func(int sockfd)
{
    struct timespec start, end;
    double tiempo_t;
    char buff[MAX];
    int num, res;
    int n;
    // Loop for the chat
    for (;;)
    {
        // marco comienzo de procesamiento
        clock_gettime(CLOCK_MONOTONIC, &start);

        bzero(buff, MAX);

        // leo mensaje de cliente y lo guardo
        read(sockfd, &num, sizeof(num));

        // // Muestro mensaje del cliente
        printf("\nNumero del cliente: %d\n", num);
        // bzero(buff, MAX);

        // Calculo factorial
        res = factorial(num);

        // Enviar respuesta al cliente
        send(sockfd, &res, sizeof(res), 0);
        printf("Respuesta (%d) enviada.\n", res);

        // marco final de cronometro
        clock_gettime(CLOCK_MONOTONIC, &end);
        tiempo_t = tiempo_transcurrido(&start, &end);

        // Enviar tiempo de respuesta al cliente
        send(sockfd, (void*)&tiempo_t, sizeof(tiempo_t), 0);
        printf("Tiempo de respuesta: %f\n", tiempo_t);
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
