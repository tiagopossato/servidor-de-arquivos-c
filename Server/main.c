#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "netinet/in.h"
#include "sys/socket.h"
#include "sys/types.h"
#include "arpa/inet.h"
#include "netinet/in.h"

#define ERRO -1
#define TAMMAX 1024 //tamanho maximo da string

/*Pasta com os arquivos */
char pasta[256];

/**
 * Thread para controlar a conexão com cada novo cliente conectado no servidor
 * @param id O ID do novo cliente
 * @return 
 */
void *Recebe(void *sck)
{
    int *socket = (int *)sck;

    char *msgrecv = malloc(TAMMAX);
    char *arquivo = malloc(1024);
    char *linha = malloc(1024);
    int qtd = 0;

fprintf(stdout, "Socket: %d\n", socket);

    bzero(msgrecv, strlen(msgrecv));

    qtd = recv(socket, msgrecv, TAMMAX, 0); //recebe os dados no socket
    if (qtd <= 0)                           //verifica se é para sair
    {
        printf("\nConexão fechada.\n");
        close(socket);
        free(msgrecv);
        free(arquivo);
        free(linha);
        return;
    }
  

    sprintf(arquivo, "%s%s\0", pasta, msgrecv);
    fprintf(stdout, "%s solicitado.\n", arquivo);

    FILE *arq = fopen(arquivo, "r");
    if (arq == NULL)
    {
        send(socket, "Arquivo de não encontrado!", strlen("Arquivo de não encontrado!"), 0);
        printf("Arquivo de não encontrado!");
        close(socket);
        free(msgrecv);
        free(arquivo);
        free(linha);
        return;
    }
    while((fgets(linha, sizeof(linha), arq))!=NULL){
        //le uma linha do arquivo
        send(socket, linha, strlen(linha), 0);
        bzero(linha, strlen(linha));
    }
    //fecha tudo
    fclose(arq);                 
    close(socket);
    free(msgrecv);
    free(arquivo);
    free(linha);
    pthread_exit(NULL);
}

void main(int argc, char *argv[])
{
    char *ip = "127.0.0.1";
    int porta;
    int i;

    // printf("argc = %d\n", argc);
    // for (i = 0; i < argc; ++i)
    // {
    //     printf("argv[ %d ] = %s\n", i, argv[i]);
    // }
    if (argc != 3)
    {
        printf("Utilize: './servidor 1234 /tmp/repositorio'\n");
        exit(-1);
    }
    /*Pega a porta nos parâmetros passados*/
    porta = atoi(argv[1]);
    sprintf(pasta, "%s", argv[2]);


    /*cria a estrutura para o socket local*/
    struct sockaddr_in local, network;
    /*Variavel que armazenará o tamanho da estrutura*/
    socklen_t strucsize;
    int status, resp, sock, newsock;
    /*Quantidade de threads que o servidor irá criar*/
    int num_threads = 256;
    /*Vetor que armazenará as threads*/
    pthread_t thread[num_threads];

    /*Cria o socket*/
    sock = socket(PF_INET, SOCK_STREAM, 0);
    /*Verifica se foi criado*/
    if (sock == ERRO)
    {
        perror("Socket");
        exit(0);
    }
    /*Preenche a estrutura com zeros*/
    bzero((char *)&local, sizeof(local));
    /*Seta os valores da estrutura do socke local*/
    local.sin_family = PF_INET;             //Protocolo IP
    local.sin_port = htons(porta);          //Porta
    local.sin_addr.s_addr = inet_addr(ip);  //IP
    strucsize = sizeof(struct sockaddr_in); //pega o tamanho da estrutura

    /*associa o socket à porta */
    resp = bind(sock, (struct sockaddr *)&local, strucsize);
    if (resp == ERRO)
    {
        perror("Bind");
        exit(0);
    }

    /*Aguarda conexão do cliente*/
    /* O segundo parametro indica quantas conexões serão colocadas na fila 
     * antes da próxima ser recusada*/
    listen(sock, 10);

    /*loop para o accept*/
    while (1)
    {
        puts("Aguardando conexão...");
        //Recebe nova conexão
        if ((newsock = accept(sock, (struct sockaddr *)&network, &strucsize)) < 0)
        {
            perror("accept");
            exit(1);
        }
        else //caso não deu erro na conexão
        {
            printf("\nRecebendo conexao de: %s, socket: %d\n", inet_ntoa(network.sin_addr), newsock);
            //cria nova thread passando como parametro o numero do cliente
            status = pthread_create(&thread[i], (void *)NULL, Recebe, (void *)newsock);
            if (status)
            { //ocorreu erro
                perror("pthread_create");
                exit(-1);
            }
            i++;
        }
    }

    //esperando término das threads
    for (i = 0; i < num_threads; i++)
    {
        printf("Aguardando thread numero (id): %d\n", i);
        status = pthread_join(thread[i], NULL);
        if (status)
        {
            perror("pthread_join");
            exit(-1);
        }
    }
}