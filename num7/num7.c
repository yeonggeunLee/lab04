#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 3490
#define QLEN 10
#define BUF_SIZE 1024

void *handle_clnt(int sockfd);
void send_err(int sockfd);
void send_msg(int sockfd);

int main(int argc, char *argv[]) {
    int sockfd, new_fd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int adresslen;
    fd_set readfds, activefds;
    int i, maxfd = 0, numbytes;
    char buf[100];
    
    if ((sockfd = socket (AF_INET, SOCK_STREAM, 0))	< 0) {
        perror("socket() failed");
        exit(1);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind (sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr))	< 0) {
        perror("bind() failed");
        exit(1);
    }
    
    if (listen(sockfd, QLEN) < 0) {
        fprintf(stderr,"listen failed\n");
        exit(1);
    }
    adresslen = sizeof(client_addr);
    
    FD_ZERO (&activefds);
    FD_SET (sockfd, &activefds);
    maxfd = sockfd;

    fprintf( stderr, "서버 시작.\n");
    while (1) {
        printf("서버: 연결 기다리는 중..., %d\n", maxfd);
        readfds = activefds;
        if(select (maxfd + 1, &readfds, NULL, NULL, NULL) < 0)
        {
            perror ("select");
            exit (EXIT_FAILURE);
        }

        for (i = 0; i <= maxfd; i++) {
            if (FD_ISSET (i, &readfds)) {
                if (i == sockfd) {
                    if ( (new_fd=accept(sockfd, (struct sockaddr *)&client_addr, &adresslen)) < 0) {
                        fprintf(stderr, "accept failed\n");
                        exit (1);
                    }
                    FD_SET(new_fd, &activefds);
                    if (new_fd > maxfd)
                        maxfd = new_fd;
                }
                else {
                    printf("handle client\n");
                    handle_clnt(i);

                    close(i);
                    FD_CLR(i, &activefds);
                }
            }
        }
    }
    close(sockfd);
}

void *handle_clnt(int client_sock) {
    int i;
    int recv=0, str_len=0;
    int readcnt=0;
    char msg[BUF_SIZE]; 
    char method[10];
    
    if ((str_len=read(client_sock, &msg[recv], BUF_SIZE)) == -1) {
        printf("read() error!\n");
        exit(1);
    }
    
    recv += str_len;
    strcpy(method, strtok(msg, " "));
    if (strcmp(method, "GET") != 0)
        send_err(client_sock);
    else
        send_msg(client_sock); 
    return NULL;
}

void send_err(int client_sock) {
    char protocol[] = "HTTP/1.1 400 Bad Request\r\n";
    char server[] = "Server:NetscapeEnterprise/6.0\r\n";
    char contenttype[] = "ContentType:text/html\r\n";
    char html[] = "<html><head>BAD Connection</head><body><H1>Bad Request</H1></body></html>\r\n";
    char end[]="\r\n";
    
    printf("send err\n");
    write(client_sock, protocol, strlen(protocol));
    write(client_sock, server, strlen(server));
    write(client_sock, contenttype, strlen(contenttype));
    write(client_sock, end, strlen(end));
    write(client_sock, html, strlen(html));
    fflush(fdopen(client_sock, "w"));
}

void send_msg(int client_sock) {
    char protocol[] = "HTTP/1.1 200 OK\r\n";
    char server[] = "Server:Netscape-Enterprise/6.0\r\n";
    char contentlength[] = "Content-Length: 139\r\n";
    char contenttype[] = "Content-Type: text/html; charset=utf-8\r\n";
    char html[] = "<html><head>시스템프로그래밍 과제 7번</head><body><center>\n<H1>과제 어렵다....</H1>\r\n<H2>20212952 이영근</H2></center></body></html>";
    char end[]="\r\n";

    printf("send msg, len=%ld\n", strlen(html));
    write(client_sock, protocol, strlen(protocol));
    write(client_sock, server, strlen(server));
    write(client_sock, contentlength, strlen(contentlength));
    write(client_sock, contenttype, strlen(contenttype));
    write(client_sock, end, strlen(end));
    write(client_sock, html, strlen(html));
    fflush(fdopen(client_sock, "w"));
}
