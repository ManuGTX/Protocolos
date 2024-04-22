#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h> // wait()
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#define MAX 80
#define PORT 8080
#define NUM_CONNECTIONS 5
#define SA struct sockaddr

void func(int sockfd)
{
    
    double tiempo_t;
    char buff[MAX];
    int n, res;
    for (int num = 1; num < 15; num++) {
        // bzero(buff, sizeof(buff));
        // printf("Ingrese texto : ");
        // n = 0;
        // while ((buff[n++] = getchar()) != '\n')
        //     ;
        // write(sockfd, buff, sizeof(buff));
        // bzero(buff, sizeof(buff));
        // read(sockfd, buff, sizeof(buff));
        // printf("Servidor : %s", buff);
        // if ((strncmp(buff, "exit", 4)) == 0) {
        //     printf("Cliente cierra conexión...\n");
        //     break;
        // }
        
        bzero(buff, sizeof(buff));
        
        write(sockfd, &num, sizeof(num));
        printf("\nNumero enviado: %d\n", num);

        // Recibir respuesta del servidor
        read(sockfd, &res, sizeof(res));
        printf("Respuesta del servidor: %d\n", res);

        // Recibir tiempo de respuesta del servidor
        read(sockfd, (void*)&tiempo_t, sizeof(tiempo_t));
        printf("Tiempo de respuesta del servidor: %f segundos\n", tiempo_t);

    }
}
 
int main()
{
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;
    char buff[MAX];
 
    // socket: creo socket y lo verifico
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Falla la creación del socket...\n");
        exit(0);
    }
    else
        printf("Socket creado ..\n");
    bzero(&servaddr, sizeof(servaddr));
 
    // asigno IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);
 
    // Conecto los sockets entre cliente y servidor
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    else {
        printf("Conectado al servidor..\n");

        // Crear múltiples conexiones utilizando fork()
        for (int i = 0; i < NUM_CONNECTIONS; ++i) {
            pid_t pid = fork();

            if (pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            } else if (pid == 0) { // Proceso hijo
                printf("Proceso hijo %d creando conexión...\n", getpid());

                // Enviar mensaje al servidor
                sprintf(buff, "Mensaje desde el cliente %d\n", getpid());
                write(sockfd, buff, sizeof(buff));
                printf("Proceso hijo %d: Mensaje enviado al servidor\n", getpid());

                // Recibir respuesta del servidor
                int bytes_recibidos = recv(sockfd, buff, sizeof(buff), 0);
                if (bytes_recibidos > 0) {
                    buff[bytes_recibidos] = '\0';
                    printf("Proceso hijo %d: Respuesta del servidor: %s\n", getpid(), buff);
                } else if (bytes_recibidos == 0) {
                    printf("Proceso hijo %d: Conexión cerrada por el servidor\n", getpid());
                } else {
                    perror("recv");
                }

                // Cerrar el socket y terminar el proceso hijo
                close(sockfd);
                exit(EXIT_SUCCESS);
            }
        }

        // Esperar a que todos los procesos hijos terminen
        for (int i = 0; i < NUM_CONNECTIONS; ++i) {
            wait(NULL);
        }
    }
    // Función para el chat
    // func(sockfd);
 
    //Cierro el socket
    close(sockfd);
}