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
* This is a simple HTTP Client which sends HTTP GET request for the URL mentioned in command line.

@USAGE:
        $ ./HTTPClient <URL>
*/

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<errno.h>
#include<unistd.h>
#include<string.h>
#include<netinet/in.h>
#include<fcntl.h>
#include<netdb.h>
#include<time.h>

#define REQUEST_MSGLEN        1024
#define READ_LEN              4096

struct http_req {
    char *host;
    char *path;
    char *port;
};

struct http_req *parseURL(char *url);
int connectToHTTPServer(char *host, char *port);
int makeRequest(int sock, char *path);
int getResponse(int sock, char *path);
void getTimeDifferenceInStringFormat(int epoch_time_dif, char *time_dif);

int main(int argc, char *argv[]){
    int sock;
    char *host = NULL;
    int port = -1;
    struct http_req *request;
    size_t bytes_received;
    time_t start_time, end_time;
    char time_dif[16];

    if(argc != 2){
        fprintf(stderr, "USAGE: /HTTPClient <URL>\n");
        exit(EXIT_FAILURE);
    }

    if((request = parseURL(argv[1])) == NULL){
        fprintf(stderr, "URL Not supported!\n");
        exit(EXIT_FAILURE);
    }

    time(&start_time);
    printf("connecting...\n");
    if((sock = connectToHTTPServer(request->host, request->port)) < 0){ //establish a connection to HTTP Server
        perror("Could not connect to HTTP server!\n");
        exit(EXIT_FAILURE);
    }

    printf("connected!\n");
    if(makeRequest(sock, request->path) <= 0){
        fprintf(stderr,"Request send failed!\n");
        exit(EXIT_FAILURE);
    }

    if((bytes_received = getResponse(sock, request->path)) <= 0){
        fprintf(stderr, "Could not read server response!\n");
        exit(EXIT_FAILURE);
    }

    time(&end_time);
    getTimeDifferenceInStringFormat(end_time - start_time, time_dif);
    printf("Number of bytes received: %zu\n",bytes_received);
    printf("Time taken to fetch response: %s\n",time_dif);
    exit(EXIT_SUCCESS);

}

int connectToHTTPServer(char *host, char *port){
    /*Establish a connection with the HTTP Server and return the connected socket*/

    int sockfd;
    int gai_ret;
    struct addrinfo hints;
    struct addrinfo *result, *res_p;

    hints.ai_family = AF_UNSPEC;   /* Allow IPv4 or IPv6 */
    hints.ai_socktype = 0;         /* Any type of Socket */
    hints.ai_protocol = 0;         /* Any protocol */
    hints.ai_flags = AI_CANONNAME; /* Get canonical name of the host */

    if((gai_ret = getaddrinfo(host, port, &hints, &result)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n",gai_strerror(gai_ret));
        return -1;
    }

    /* getaddrinfo(3) returns a list of address structures.
       Try each address until we successfully connect(2).
       If socket(2) (or connect(2)) fails, we (close the 
       socket and) try the next address. */

    for(res_p = result; res_p != NULL; res_p = res_p->ai_next){
        sockfd = socket(res_p->ai_family, res_p->ai_socktype,
                        res_p->ai_protocol);

        if(sockfd < 0)
            continue;

        if(connect(sockfd, res_p->ai_addr, res_p->ai_addrlen) == 0)
            break;                       /* Connected! */

        close(sockfd);
    }

    if(res_p == NULL){ /* No results */
        fprintf(stderr, "Could not connect!\n");
        return -1;
    }

    freeaddrinfo(result); /* We used server details; No longer needed in memory */
    return sockfd;
}

int makeRequest(int sockfd, char *request_path){
    char buf[REQUEST_MSGLEN];
    size_t bytes_sent       = 0;
    size_t total_bytes_sent = 0;
    size_t bytes_to_send    = 0;

    sprintf(buf, "GET %s HTTP/1.0\r\n\r\n", request_path);

    bytes_to_send = strlen(buf);
    while(total_bytes_sent < bytes_to_send){
        bytes_sent = send(sockfd, buf, strlen(buf), 0);
        total_bytes_sent += bytes_sent;
    }

    return total_bytes_sent;
}

int getResponse(int sock, char *path){
/* #TODO: Receive response and if 200 OK, start receiving file and store locally.
   *Return number of bytes received.
*/

    char buf[READ_LEN];
    size_t bytes_received   = 0;
    size_t total_bytes_received = 0;
    size_t file_data_len;
    size_t written;
    char *file_data_offset;
    char *result = NULL;
    char *filename;
    int header_len;
    FILE *resource;
    int count = 0;

    while(1){
        bytes_received = recv(sock, buf, READ_LEN, 0);

        count++;
        if(bytes_received <= 0){
            break;
        }

        result = (char*)realloc(result, bytes_received + total_bytes_received);

        memcpy(result + total_bytes_received, buf, bytes_received);
        total_bytes_received += bytes_received;
    }

    if(!is_http_status_ok(result)){
        return total_bytes_received;
    }

    header_len = getHeaderLength(result);
    if(!strcmp("/",path)){
        filename = "index.html";
    }
    else{
        filename = basename(path);
    }

    file_data_offset = result + header_len;
    file_data_len = total_bytes_received - header_len;

    resource = fopen(filename, "ab");
    if(resource == NULL){
        fprintf(stderr, "Could not create file!\n");
        return total_bytes_received;
    }

    written = fwrite(file_data_offset, 1, file_data_len, resource);
    if(written != file_data_len){
        fprintf(stderr, "Complete data could not be wrtiiten to file!\n");
    }

    fclose(resource);
    return total_bytes_received;
}

int getHeaderLength(char *content){
    const char *srch1 = "\r\n\r\n", *srch2 = "\n\r\n\r";
    char *findPos;
    int ofset = -1;

    findPos = strstr(content, srch1);
    if (findPos != NULL){
        ofset = findPos - content;
        ofset += strlen(srch1);
    }

    else{
        findPos = strstr(content, srch2);
        if (findPos != NULL)
        {
            ofset = findPos - content;
            ofset += strlen(srch2);
        }
    }
    return ofset;
}

int is_http_status_ok(char *response){
    char http[10], status_code[4], status[10];

    sscanf(response,"%s %s %s", http, status_code, status);
    return ((!strncmp("200", status_code, 3)) && (!strncmp("OK", status, 2)));
}

struct http_req *parseURL(char *url){
    //Parse URL to get host, port and resource path
    struct http_req *req;
    char *temp1, *temp2;
    int len;
    char *path = NULL;

    req = (struct http_req *)malloc(sizeof(struct http_req));

    if(!strncmp(url, "http://", strlen("http://"))){
        temp1 = strstr(url + strlen("http://"), ":");
        if(temp1 == NULL){
            return NULL;
        }

        len = temp1 - strlen("http://") - url + 1;
        req->host = (char *)malloc(len * sizeof(char));
        strncpy(req->host, url + strlen("http://"), len - 1);
        req->host[len] = '\0';

        temp2 = strstr(temp1, "/");
        if(temp2 == NULL){
            return NULL;
        }

        len = temp2 - temp1;
        req->port = (char *)malloc(len * sizeof(char));
        strncpy(req->port, temp1, len);
        req->port[len] = '\0';

        len = temp2 - url;
        req->path = (char *)malloc(len * sizeof(char));
        strncpy(req->path, temp2, len - 1);
        req->path[len] = '\0';

        return req;
    }

    else if(!strncmp(url, "https://", strlen("https://"))){
        temp1 = strstr(url + strlen("https://"), "/");
        if(temp1 == NULL){
            path = "/";
            temp1 = url + strlen(url);
        }

        len = temp1 - url - strlen("https://");
        req->host = (char *)malloc(len * sizeof(char));
        strncpy(req->host, url + strlen("https://"), temp1 - url - strlen("https://"));
        req->host[len] = '\0';

        len = temp1 - url;
        req->path = (char *)malloc(len * sizeof(char));
        if(path == NULL){
            strcpy(req->path, temp1);
        }
        else
            strcpy(req->path, path);

        req->path[len] = '\0';

        req->port = (char *)malloc(3 * sizeof(char));
        strcpy(req->port, "80");
        req->port[3] = '\0';
        return req;
    }

    return NULL;
}

void getTimeDifferenceInStringFormat(int epoch_time_dif, char *time_dif){
    /* This function takes Epoch time difference and converts it to
       HH:MM:SS fromat and stores in time_dif buffer*/

    int hours, minutes, seconds, remainig_seconds;
    struct tm temp;

    hours = epoch_time_dif / 3600;
    remainig_seconds = epoch_time_dif % 3600;
    minutes = remainig_seconds / 60;
    seconds = remainig_seconds % 60;

    temp.tm_hour = hours;
    temp.tm_min = minutes;
    temp.tm_sec = seconds;

    strftime(time_dif,10,"%T",&temp);
}
