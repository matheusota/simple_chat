#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <dirent.h>
#include <stdbool.h>

#define MAXLINE 4096
#define MAXDATASIZE 4096
#define MAXFILENAME 100
#define LISTENQ 10

//##############################################################
//wrapper functions
int Socket(int family, int type, int flags) {
	int sockfd;
	
	if ((sockfd = socket(family, type, flags)) < 0) {
		perror("socket error");
		exit(1);
	} 
	
	else
		return sockfd;
}

void Inet_pton(int af, const char *src, void *dst){
	if (inet_pton(af, src, dst) <= 0) {
		perror("inet_pton error");
		exit(1);
	}
}

void Fputs(char *msg){
	if (fputs(msg, stdout) == EOF) {
		perror("error fputs");
		exit(1);
	}
}

void Bind(int sockfd, const struct sockaddr *addr, socklen_t len){
    if (bind(sockfd, addr, len) == -1) {
        perror("bind");
        exit(1);
    }
}

void Listen(int sockfd, int backlog){
    if (listen(sockfd, backlog) == -1) {
        perror("listen");
        exit(1);
    }
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *len){
    int connfd;
    
    if ((connfd = accept(sockfd, (struct sockaddr *) addr, len)) == -1 ) {
        perror("accept");
        exit(1);
    }
    
    return connfd;
}

void Connect(int sockfd, struct sockaddr *serv_addr, socklen_t len){
	if (connect(sockfd, serv_addr, len) < 0) {
		perror("connect error");
		exit(1);
	}
}


void Close(int sockfd){
    if (close(sockfd) == -1 ) {
        perror("close");
        exit(1);
    }
}

ssize_t Recvfrom(int socket, void *restrict buffer, size_t length,
       int flags, struct sockaddr *restrict address,
       socklen_t *restrict address_len){
	
	ssize_t r = recvfrom(socket, buffer, length, flags, address, address_len);
    
    if (r == -1) {
        perror("listen");
        exit(1);
    }
    
    else
		return r;
}

ssize_t Sendto(int socket, const void *message, size_t length,
       int flags, const struct sockaddr *dest_addr,
       socklen_t dest_len){
	
	ssize_t r = sendto(socket, message, length, flags, dest_addr, dest_len);
    
    if (r == -1) {
        perror("listen");
        exit(1);
    }
    
    else
		return r;
}

int Select(int nfds, fd_set *restrict readfds, fd_set *restrict writefds, fd_set *restrict errorfds, struct timeval *restrict timeout){
	int r = select(nfds, readfds, writefds, errorfds, timeout);
	
	if(r > 0)
		return r;
	else{
		perror("error select");
		exit(1);
	}
}

//##############################################################
//helper functions
int max (int a, int b){
	if(a > b)
		return a;
	else
		return b;
}

//check if the process with a given name is running
bool findProcess(char *name){
	const char* directory = "/proc";
	size_t      taskNameSize = 1024;
	char*       taskName = calloc(1, taskNameSize);

	DIR* dir = opendir(directory);

	if (dir)
	{
		struct dirent* de = 0;

		while ((de = readdir(dir)) != 0)
		{
			if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
				continue;

			int pid = -1;
			int res = sscanf(de->d_name, "%d", &pid);

			if (res == 1)
			{
				// we have a valid pid

				// open the cmdline file to determine what's the name of the process running
				char cmdline_file[1024] = {0};
				sprintf(cmdline_file, "%s/%d/cmdline", directory, pid);

				FILE* cmdline = fopen(cmdline_file, "r");

				if (getline(&taskName, &taskNameSize, cmdline) > 0)
				{
					// is it the process we care about?
					if (strstr(taskName, name) != 0)
					{
						return true;
					}
				}

			fclose(cmdline);
			}
		}

		closedir(dir);
	}

	free(taskName);
	return false;
} 

// Get ip from domain name
bool hostname_to_ip(char * hostname , char* ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
         
    if ( (he = gethostbyname( hostname ) ) == NULL) 
    {
		return false;
    }
 
    addr_list = (struct in_addr **) he->h_addr_list;
     
    for(i = 0; addr_list[i] != NULL; i++) 
    {
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return true;
    }
     
    return false;
}

//checa se o nome eh um ip (de maneira simplificada)
bool isIp(char *name){
	int n = strlen(name);
	int i;
	
	for(i = 0; i < n; i++){
		if(!((name[i] - 48 >= 0 && name[i] - 48 <= 9) || name[i] == '.'))
			return false;
	}
	
	return true;
}


//main function
int main(int argc, char **argv) {	
	int    sockfd, n;
	char   recvline[MAXLINE + 1];
	struct sockaddr_in servaddr;
	char   buf[MAXDATASIZE];
	socklen_t len = sizeof(struct sockaddr);
	char file_name[MAXFILENAME];
	pid_t pid;
	
	//numero errado de argumentos passados pela linha de comando
	if (argc != 3) {
		printf("modo de utilizacao:\n");
		printf("./cliente [IP_servidor | nome_servidor] [porta_servidor]\n");
		exit(1);
	}

	//Exibe dados do servidor ao qual cliente esta se conectando
	//printf("Conecting to server...\n");
	//printf("IP: %s\n", argv[1]);
	//printf("Porta: %s\n", argv[2]);
	
	//inicializa um socket e coloca sua referencia (file descriptor) em sockfd
	sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

	//preenche com zeros a estrutura servaddr
	bzero(&servaddr, sizeof(servaddr));

	//informa que as mensagens trocadas no socket utilizam o IPV4
	servaddr.sin_family = AF_INET;

	//informa a porta utilizada pelo socket, convertendo para big endian se necessario
	servaddr.sin_port   = htons(atoi(argv[2]));

	//checa se recebeu um ip ou o nome
	char ip[100];
	strcpy(ip, argv[1]);
	
	if(!isIp(argv[1])){
		//checa se tem um processo local com o nome
		if(findProcess(argv[1]))
			strcpy(ip, "127.0.0.1");
		
		//checa se existe esse nome usando o servico dns	
		else if(!hostname_to_ip(argv[1], ip)){
			printf("Nao encontrei o servidor especificado.\n");
			return 0;
		}
	}
	
	//coloca o ip recebido por linha de comando na estrutura servaddr
	Inet_pton(AF_INET, ip, &servaddr.sin_addr);
	
	//variaveis que serao parametros para o select
	int maxfdp1;
    fd_set rset;
    int eof_stdin = 0;
    
    //envia uma mensagem vazia para o servidor para receber a porta correta
    bzero(buf, MAXDATASIZE);
    Sendto(sockfd, buf, 1, 0, (struct sockaddr *) &servaddr, len);
    
    n = Recvfrom(sockfd, recvline, MAXLINE, 0, (struct sockaddr *) &servaddr, &len);
	unsigned short new_port;
	
	recvline[n] = 0;
	sscanf(recvline, "%hu", &new_port);
    
    printf("Porta dedicada: %hu\n", ntohs(new_port));
    
    //atualiza a porta utilizada pelo socket, convertendo para big endian se necessario
	servaddr.sin_port = htons(new_port);
    
    //envia uma nova mensagem vazia para o servidor para receber o bem vindo de volta
    bzero(buf, MAXDATASIZE);
    Sendto(sockfd, buf, 1, 0, (struct sockaddr *) &servaddr, len);
    
    //envia para o servidor
    while(1){
		//seta os descritores do stdin e do socket
		if(!eof_stdin)
			FD_SET(fileno(stdin), &rset);
		FD_SET(sockfd, &rset);
        
        //seta um limitante para o numero dos descritores (isso ajuda no desempenho do select)
        maxfdp1 = max(fileno(stdin), sockfd) + 1;
        
        //chama select, quando ele voltar, somente os descritores que receberam algo terao 1 no fd_set
        Select(maxfdp1, &rset, NULL, NULL, NULL);
		
        //stdin recebeu algo para ler
        if (FD_ISSET(fileno(stdin), &rset)){
			bzero(buf, MAXDATASIZE);
			read(fileno(stdin), buf, MAXDATASIZE);
			
			char subbuf[6];
			strncpy(subbuf, buf, 6);
			
			//ativa transferencia de arquivo
			if(strcmp(subbuf, "\\file ") == 0){
				bzero(file_name, MAXFILENAME);
				strncpy(file_name, buf + 6, strlen(buf) - 7);
				
				//envia mensagem para o servidor para abrir conexao tcp no amigo
				sprintf(buf, "\\file1\n");
				Sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *) &servaddr, len);
			}
			
			else{
				Sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *) &servaddr, len);
				
				//saiu do chat
				if(strcmp(buf, "\\exit\n") == 0)
					break;
			}
		}
		
		//socket recebeu algo para ler
        if (FD_ISSET(sockfd, &rset)){
			bzero(recvline, MAXLINE);
			
			n = Recvfrom(sockfd, recvline, MAXLINE, 0, (struct sockaddr *) &servaddr, &len);
			
			if(n > 0){
				char cmd[100];
				unsigned long ip;
				unsigned short port;
				
				recvline[n] = 0;
				sscanf(recvline, "%s %lu %hu", cmd, &ip, &port);
				
				//transferencia de arquivo: precisa abrir conexao para receber
				if(strcmp(cmd, "\\file_receiver") == 0){
					int listenfd, connfd;
					struct sockaddr_in myaddr;
					
					//coloca endereco local passado em myaddr
					myaddr.sin_family = AF_INET;
					myaddr.sin_port = port;
					myaddr.sin_addr.s_addr = ip;
					
					//cria o socket e associa ao endereco cadastrado no servidor
					listenfd = Socket(AF_INET, SOCK_STREAM, 0);
					Bind(listenfd, (struct sockaddr *)&myaddr, len);
					
					//coloca o socket para escutar novas requisicoes
					Listen(listenfd, LISTENQ);
					
					//avisa o servidor para ele avisar o cliente
					sprintf(buf, "\\file2\n");
					Sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *) &servaddr, len);
					
					//aceita uma conexao nova e cria um novo socket para ela
					connfd = Accept(listenfd, (struct sockaddr *) &myaddr, &len);
        
					//esse bloco eh executado somente no filho
					if((pid = fork()) == 0) {
						FILE *fp;
						fp = fopen ("output.txt", "w");
						Close(listenfd); //filho deve fechar o listening socket
						
						//agora processamos a requisicao
						while(1){					
							//recebe as mensagens do servidor
							while ( (n = read(connfd, buf, MAXDATASIZE)) > 0) {
								buf[n] = 0;
								fprintf(fp, buf);
								break;
							}
							
							//problema de leitura
							if (n < 0) {
								perror("error reading socket");
								exit(1);
							}
							
							if(n == 0)
								break;
						}
						
						Close(connfd); //filho terminou de processar, connection socket pode ser fechado
						exit(0); //termina
					}
					
					printf("Arquivo recebido e salvo como 'output.txt' .\n");
					Close(connfd); //pai deve fechar o connection socket
					Close(listenfd);
				}
				
				//transferencia de arquivo: precisa abrir conexao para enviar
				else if(strcmp(cmd, "\\file_sender") == 0){
					int writefd;
					struct sockaddr_in cliaddr;
					
					//coloca endereco local passado para em myaddr
					cliaddr.sin_family = AF_INET;
					cliaddr.sin_port = port;
					cliaddr.sin_addr.s_addr = ip;
					
					//esse bloco eh executado somente no filho
					if((pid = fork()) == 0) {
						//inicializa socket
						writefd = Socket(AF_INET, SOCK_STREAM, 0);
						
						//tenta realizar a conexao
						Connect(writefd, (struct sockaddr *) &cliaddr, len);
						
						FILE *fp;
						fp = fopen (file_name, "r");						
						
						//envia para o servidor
						while(fgets(buf, MAXDATASIZE, fp) != NULL){
							n = write(writefd, buf, strlen(buf));
							
							if (n < 0) 
								printf("Falha ao escrever para o socket\n");
								
							bzero(buf, MAXDATASIZE);
						}
						
						fclose(fp);
						Close(writefd);
						printf("Arquivo enviado.\n");
						exit(0); //termina
					}
				}
				
				else{
					printf("[RECEBIDO:] ");
					Fputs(recvline);
				}
			}
		}
	}

	exit(0);
}
