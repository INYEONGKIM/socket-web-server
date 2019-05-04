/*
* Computer Network project 1 (Concurrent Web Server using BSD Sockets)
* Name : INYEONG KIM
* Major : Division of Computer Science, Softerware Major
* Student ID : 2016015878
*/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h> 

#define BUFFER_SIZE 256
#define BACKLOG 10 //Maximum number of clinet

void error(char *msg){
    perror(msg);
    exit(1);
} //error

char *getContentType(char *contentPath){
    char *extents[] = { ".html", ".jpeg", ".gif", ".mp3", ".pdf", ".ico" };
    char *contentType[] = { "text/html", "image/jpeg", "image/gif", "audio/mp3", "application/pdf", "image/x-icon" };
    char *res = contentType[0];
    int len = (int)(sizeof(extents) / sizeof(extents[0]));

    for(int i=0; i<len; i++){
        if(strstr(contentPath, extents[i]) != NULL){
            res = (char *)malloc(strlen(contentType[i]) + 1);
            strcpy(res, contentType[i]);
            break;
        }
    }
    return res;
} //getContentType

int main(int argc, char **argv){
    int sockfd, newsockfd;
    int portno;
    int n;
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE]; //response message

    struct sockaddr_in serv_addr, cli_addr;

    if(argc < 2){
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    
    /* socket */
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        error("ERROR opening socket");
    }

    /* ctrl+c : release current port */
    int reuseAddress;
    reuseAddress = 1;
    setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseAddress, sizeof(reuseAddress) );

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* bind */
    if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        error("ERROR on binding");
    }

    /* start server */
    listen(sockfd, BACKLOG);
    socklen_t clilen = sizeof(cli_addr);
    while(1){
        /* accept */
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if(newsockfd < 0){
            error("ERROR on accept");
        }

        bzero(buffer,BUFFER_SIZE);
        bzero(response,BUFFER_SIZE);
        if(read(newsockfd, buffer, BUFFER_SIZE) < 0){
            error("ERROR reading from socket");
        }
        printf("# request : \n%s\n", buffer);
        char *token = strtok(buffer, " "); //GET
        if (token != NULL){
            char *rawPath = strtok(NULL, " ");
            printf("# rawPath : %s\n", rawPath);
            if ( rawPath != NULL ) {
                char *path;
                /* set path */
                if(strcmp(rawPath, "/") == 0){ //root
                    path = (char *)malloc(strlen("./index.html") + 1);
                    strcpy(path, "./index.html");
                }else{
                    path = (char *)malloc(strlen(rawPath) + 1);
                    sprintf(path, ".%s", rawPath);
                }
                
                char *contentType = getContentType(path); // text/html
                printf("# path : %s\n", path);
                printf("# contentType : %s\n", contentType);

                int file = open(path, O_RDONLY);
                if(file < 0){ //not exist
                    file = open("./404.html", O_RDONLY);
                    contentType = (char *)malloc(strlen("text/html") + 1);
                    strcpy(contentType, "text/html");
                }

                // get file size and reset cursor
                int fileSize = lseek(file, 0, SEEK_END);
                lseek(file, 0, SEEK_SET);
                
                /* make response */
                sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %d\r\nConnection: Alive\r\n\r\n", contentType, fileSize);
                printf("\n\n# response : \n%s\n\n", response);
                n = write(newsockfd, response, strlen(response));
                if (n < 0) error("ERROR writing to socket");

                while ((fileSize = read(file, buffer, BUFFER_SIZE)) > 0){
                    n = write(newsockfd, buffer, BUFFER_SIZE);
                    if (n < 0) error("ERROR writing to socket");
                }
                close(file);
                printf("===========================\n\n");    
            } // rawPath
        } //token
    } // while(1)
    close(newsockfd);
    close(sockfd);

    return 0;
} // main