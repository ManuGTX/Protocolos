#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#define MAX 80
#define PORT 8080
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
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr))
        != 0) {
        printf("Falla de conexión con servidor...\n");
        exit(0);
    }
    else
        printf("Conectado al servidor..\n");
 
    // Función para el chat
    func(sockfd);
 
    //Cierro el socket
    close(sockfd);
}