#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "string.h"
#include "errno.h"
#include "sys/socket.h"
#include "sys/types.h"
#include "arpa/inet.h"
#include "netdb.h"
#include "netinet/in.h"
#include <pthread.h>
#include <bits/signum.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>

#define ERRO -1
#define TAMMAX 1024 //tamanho maximo da string

int main(int argc, char *argv[]) {

    char ip[16];
    int porta;
    char arquivo[1024];
    char msgrecv[1024];


    if (argc != 4)
    {
        printf("Utilize: './cliente 127.0.0.1 1234 /tmp/repositorio'\n");
        exit(-1);
    }
    /*Pega a porta nos parâmetros passados*/
    sprintf(ip, "%s", argv[1]);
    porta = atoi(argv[2]);
    sprintf(arquivo, "%s", argv[3]);

    //cria a variavel para a estrutura o número da porta e o número IP
    struct sockaddr_in network;
    int sock, resp, strucsize, fecha = 0;
    char *msgsend = malloc(TAMMAX);

    pthread_t thread;
    int threadStatus;


    /*CRIAR UM NOVO SOCKET COM OS PARAMETROS:
    -'AF_INET'=Endereçamento IPv4
    -'SOCK_STREAM'=Este tipo de socket permite uma conexão bidirecional,
     seqüêncial e com fluxo único de dados. Quando se trata da famàlia da internet
     (PF_INET) o tipo SOCK_STREAM se refere ao protocolo TCP
     (Protocolo de Controle de Transmissão ou Transmission Control Protocol). O
     protocolo TCP é conhecido por sua confiabilidade.
    -'0'=protocolo IP, definido em /etc/protocols que remete à
     * http://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml
     ->a variável sock vai ser o numero do socket criado<-*/
    sock = socket(AF_INET, SOCK_STREAM, 0);

    //se o socket retornar -1 (ERRO), então avisa e finaliza
    if (sock == ERRO) {
        perror("Socket");
        exit(0);
    }

    //preenche a estrutura com zeros
    bzero((char *) &network, sizeof (network));

    //coloca o tipo de protocolo IPv4 na variável
    network.sin_family = PF_INET;

    /*converter um número expresso em Host Byte Order (no qual o primeiro byte é o
     significativo) para Network Byte Order (primeiro byte menos significativo).
     Essa conversão se faz necessária por questões de compatibilidade.
    http://www.forum-invaders.com.br/vb/showthread.php/20396-Sockets-em-C-Linux-Parte-1 */
    network.sin_port = htons(porta);

    /*obtém um IP no formato string e o converte para o seu respectivo valor
     numérico na forma de Network Byte Ordem - que é o tipo requerido pela estrutura
     sockaddr. A função inet_addr() pode trabalhar apenas sob endereços IP.*/
    network.sin_addr.s_addr = inet_addr(ip);

    //pega o tamanho da variável
    strucsize = sizeof (network);

    /*inicia a conexão no socket 'sock' */
    resp = connect(sock, (struct sockaddr *) &network, strucsize);
    if (resp == ERRO) {
        perror("Connect");
        exit(0);
    }

    /*Seta opções do socket*/
    int optval = 65535;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval)) == -1)
        printf("setsockopt() failed");

    /*Imprime que foi conectado*/
    fprintf(stdout, "Conectado em %s\n", ip);
    fprintf(stdout, "Solicitando: %s\n", arquivo);
    //envia nome do arquivo para o servidor
    send(sock, arquivo, strlen(arquivo), 0);

    //recebe o arquivo
    bzero(msgrecv, strlen(msgrecv));
    while (1) {
        if (recv(sock, msgrecv, TAMMAX, 0) > 0) {
            fprintf(stdout, "%s", msgrecv);
            bzero(msgrecv, strlen(msgrecv));
        } else
        break;
    }
    close(sock);
    
    //fecha o arquivo
    exit(0);
}
