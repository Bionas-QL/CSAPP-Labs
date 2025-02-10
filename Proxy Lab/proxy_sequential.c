#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *pro_conn_hdr = "Proxy-Connection: close\r\n";




void doit(int fd);
void create_request(char *request_to_server, char *host, char *path, rio_t *rio);
void parse_uri(char *uri, char *host, char *path, char *port);
void get_filetype(char *filename, char *filetype);
void clienterror(int fd, char *cause, char *errnum,
		 char *shortmsg, char *longmsg);

int main(int argc, char **argv)
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    /* Check command line args */
    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(1);
    }

    if ((listenfd = Open_listenfd(argv[1])) < 0) {
    	fprintf(stderr, "open listenfd failed\n");
		exit(1);
    }
    while (1) {
	clientlen = sizeof(clientaddr);
	connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE,
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
	doit(connfd);
	Close(connfd);
    }
}

/*
 * doit - handle one HTTP request/response transaction
 */
/* $begin doit */
void doit(int client_fd)
{
	int server_fd;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char host[MAXLINE], path[MAXLINE], port[MAXLINE];
	char request_to_server[MAXLINE];
    rio_t client_rio, server_rio;


    Rio_readinitb(&client_rio, client_fd);
    if (!Rio_readlineb(&client_rio, buf, MAXLINE))  //line:netp:doit:readrequest
        return;
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);       //line:netp:doit:parserequest
    if (strcasecmp(method, "GET")) {                     //line:netp:doit:beginrequesterr
        clienterror(client_fd, method, "501", "Not Implemented",
                    "Proxy does not implement this method");
        return;
    }                                                    //line:netp:doit:endrequesterr

    /* Parse URI from GET request */
	strcpy(port, "80");
    parse_uri(uri, host, path, port);       //line:netp:doit:staticcheck
	create_request(request_to_server, host, path, &client_rio);

	if ((server_fd = Open_clientfd(host, port)) < 0) {
		fprintf(stderr, "Proxy can not connect to the server\n");
		return;
	}

	Rio_readinitb(&server_rio, server_fd);
	Rio_writen(server_fd, request_to_server, strlen(request_to_server));
	int num;
	while ((num = Rio_readlineb(&server_rio, buf, MAXLINE)) > 0) {
		printf("Proxy received %d bytes from the server\n", num);
		Rio_writen(client_fd, buf, num);
	}
	close(server_fd);
}
/* $end doit */



/*
 * parse_uri - parse URI
 *
 */
/* $begin parse_uri */
void parse_uri(char *uri, char *host, char *path, char *port) {
    char *ptr, *temp;

    ptr = strstr(uri, "//");
	ptr = ptr? ptr + 2: uri;
	if (strstr(ptr, ":") == NULL) {
		temp = strstr(ptr, "/");
		if (temp == NULL) {
			sscanf(ptr, "%s", host);
		} else {
			*temp = '\0';
			sscanf(ptr, "%s", host);
			*temp = '/';
			sscanf(temp, "%s", path);
		}
	} else {
		temp = strstr(ptr, ":");
		*temp = '\0';
		sscanf(ptr, "%s", host);
		temp++;
		ptr = temp;
		temp = strstr(ptr, "/");
		if (temp == NULL) {
			sscanf(ptr, "%s", port);
		} else {
			*temp = '\0';
			sscanf(ptr, "%s", port);
			*temp = '/';
			sscanf(temp, "%s", path);
		}

	}
}
/* $end parse_uri */

void create_request(char *request_to_server, char *host, char *path, rio_t *rp) {
	char buf[MAXLINE], additional_hdr[MAXLINE], request[MAXLINE], host_hdr[MAXLINE];
	sprintf(host_hdr, "Host: %s\r\n", host);

	while (Rio_readlineb(rp, buf, MAXLINE) > 0) {
		if (!strcmp(buf, "\r\n"))
			break;
		if (!strncasecmp(buf, "HOST", 4)) {
			strcpy(host_hdr, buf);
			continue;
		}
		if (!strncasecmp(buf, "User-Agent", 10))
			continue;
		if (!strncasecmp(buf, "Connection", 10))
			continue;
		if (!strncasecmp(buf, "Proxy-Connection", 16))
			continue;
		strcat(additional_hdr, buf);
	}

	sprintf(request, "GET %s HTTP/1.0\r\n", path);
	sprintf(request_to_server, "%s%s%s%s%s%s%s",
		request, host_hdr, user_agent_hdr, conn_hdr, pro_conn_hdr, additional_hdr, "\r\n");
}


/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum,
		 char *shortmsg, char *longmsg)
{
    char buf[MAXLINE];

    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* Print the HTTP response body */
    sprintf(buf, "<html><title>Tiny Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Tiny Web server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}
/* $end clienterror */