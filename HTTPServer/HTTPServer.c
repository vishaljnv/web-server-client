/*
 * San Jose State University
 * Department of Computer Engineering
 * 207 - Network Programming and Application
 * Lab Assignment-III

 * Group 4
   * Akhilesh
   * Karthikeya Rao
   * Vishal Govindraddi Yarabandi
 
NOTES:
* This is a simple HTTP Server which only serves HTTP GET rquests for a file.

@USAGE:
    $ ./HTTPServer <Resource Directory> [Listener Port]
*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<fcntl.h>
#include<signal.h>
#include<dirent.h>

#define REQUEST_MSGLEN                  1024

#define DEFAULT_PORT                    9000
#define BACKLOG                         5

#define SERVER_STRING "Server: CMPE207-Group-4\r\n"

int create_passive_socket(int port);
int service_client(int client, char *resouce_dir);

void send_http_ok(int client, const char *content_type);
void send_http_not_found(int client, const char *url);
void send_method_not_implemeted(int client);

void serve_file(int client, const char *filepath, const char *url);
const char *get_file_type(const char *file);

void reaper(int signal);

int main(int argc, char *argv[]){
    int port = DEFAULT_PORT;
    int server, client;
    int enable = 1;
    int pid;
    DIR *dirptr;
    char *dir = NULL;

    if(argc < 2){
        fprintf(stderr, "USAGE: ./HTTPServer <Rsource Directory> [Listener Port]\n");
        exit(EXIT_FAILURE);
    }

    dir = argv[1];
    if(argc > 2){
        port = atoi(argv[2]);
    }

    if((dirptr = opendir(dir)) == NULL){
        printf("Directory Not Found!\n");
        exit(EXIT_FAILURE);
    }

    server = create_passive_socket(port);
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)); //just want to be able to run immediately after Ctrl+C

    signal(SIGCHLD, reaper);  //designating a signal handler for child exit
    while(1){
        if((client = accept(server, NULL, NULL)) < 0){  //block for accepting incoming connections
            perror("accept error: ");
            exit(EXIT_FAILURE);
        }

        if((pid = fork()) > 0){ //parent here
            close(client); //close connected socket
        }
        else if( pid == 0){                    //child here
            close(server);  //close listening socket
            exit(service_client(client, dir));  //service the client
        }
        else{
            perror("fork failed!");
            close(client); //close connected socket
        }
    }
}

int create_passive_socket(int port){
    int sockfd;
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((short)port);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        perror("socket error!");
        exit(EXIT_FAILURE);
    }

    if(bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind error!");
        exit(EXIT_FAILURE);
    }

    listen(sockfd, BACKLOG);
    return sockfd;
}

int service_client(int client, char *resource_dir){
    /*Receive a client request, get the method and serve if GET, send response 501 otherwise*/

    char request[REQUEST_MSGLEN + 1];
    char path[1024];
    char method[10], http[9];
    char filepath[1024];

    recv(client, request, REQUEST_MSGLEN, 0);         //get request from the client
    printf("%s\n",request);
    sscanf(request, "%s %s %s", method, path, http);  //parse HTTP method, resource path and HTTP tag
    sprintf(filepath,"%s/%s", resource_dir, path + 1);

    printf("Requested URL: %s\n",path);
    if(strcasecmp("GET",method)){
        send_method_not_implemeted(client);
    }
    else{
        serve_file(client, filepath, path);
    }
}

void send_method_not_implemeted(int client){
    char buf[1024];

    sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
    send(client, buf, strlen(buf), 0);

    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);

    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);

    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);

    sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
    send(client, buf, strlen(buf), 0);

    sprintf(buf, "</TITLE></HEAD>\r\n");
    send(client, buf, strlen(buf), 0);

    sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
    send(client, buf, strlen(buf), 0);

    sprintf(buf, "</BODY></HTML>\r\n");
    send(client, buf, strlen(buf), 0);

    sprintf(buf, "\r\n\r\n");
    send(client, buf, strlen(buf), 0);
}

void serve_file(int client, const char *filepath, const char *url){
    char buf[1024];
    const char *content_type = NULL;
    FILE *resource = NULL;
    size_t bytes_read = 0;
    
    resource = fopen(filepath, "rb");
    content_type = get_file_type(filepath);

    if(resource == NULL){
        send_http_not_found(client, url);
        return;
    }
    else{
        send_http_ok(client, content_type);
        bytes_read = fread(buf, 1, sizeof(buf), resource);

        while(!feof(resource)){
            send(client, buf, bytes_read, 0);
            bytes_read = fread(buf, 1, sizeof(buf), resource);
        }
    }

    fclose(resource);
}

void send_http_ok(int client, const char *content_type){
    char buf[1024];

    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: %s\r\n",content_type);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n\r\n");
    send(client, buf, strlen(buf), 0);
}

void send_http_not_found(int client, const char *url){
    char buf[1024];

    sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
    send(client, buf, strlen(buf), 0);

    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);

    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);

    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);

    sprintf(buf, "<HTML><TITLE>Error 404(Not Found)!!!</TITLE>\r\n");
    send(client, buf, strlen(buf), 0);

    sprintf(buf, "<BODY><P>404. That's an error.</P>\r\n");
    send(client, buf, strlen(buf), 0);

    sprintf(buf, "<P>The requested URL %s was not found on this server.\r\n",url);
    send(client, buf, strlen(buf), 0);

    sprintf(buf, "</BODY></HTML>\r\n");
    send(client, buf, strlen(buf), 0);

    sprintf(buf, "\r\n\r\n");
    send(client, buf, strlen(buf), 0);
}

const char *get_file_type(const char *file){
    char *temp;
    if((temp = strstr(file, ".html")) != NULL){
        return "text/html";
    }
    else if((temp = strstr(file, ".pdf")) != NULL){
        return "application/pdf";
    }
    else if((temp = strstr(file, ".txt")) != NULL){
        return "text/plain";
    }
}

void reaper(int signal){

    int status;
    while(wait3(&status, WNOHANG, NULL) >= 0)
        ;/*do nothing but wait*/
}
