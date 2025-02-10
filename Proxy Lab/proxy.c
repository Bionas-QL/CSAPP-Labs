#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
// Number of blocks allowed in the cache
#define MAX_NUM_BLOCK 10

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *pro_conn_hdr = "Proxy-Connection: close\r\n";

// Building block of the cache
typedef struct {
	char *uri;
	char *result;
	unsigned long timestamp;
	int uri_size;
	int res_size;
} cache_block;

static cache_block cache[MAX_NUM_BLOCK];
// Current time stamp
static unsigned long current_stamp = 1;
// Current size of the cache
static int current_used_size = 0;
// Semaphores used for implementing reader-favor locks
static int readcnt = 0;
static sem_t mutex;
static sem_t w;

void init(void);
int check_in_cache(char *uri, char *res_buf);
void write_to_cache(char *uri, char *res_buf, int res_size);
void doit(int client_fd);
void *thread(void *vargp);
void create_request(char *request_to_server, char *host, char *path, rio_t *rio);
void parse_uri(char *uri, char *host, char *path, char *port);
void sigpipe_handler(int sig);
void clienterror(int fd, char *cause, char *errnum,
		 char *shortmsg, char *longmsg);

int main(int argc, char **argv)
{
    int listenfd;
	int *connfd_ptr;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
	pthread_t tid;
	// Ignore the SIGPIPE
	Signal(SIGPIPE,  sigpipe_handler);
    /* Check command line args */
    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(1);
    }

    if ((listenfd = Open_listenfd(argv[1])) < 0) {
    	fprintf(stderr, "open listenfd failed\n");
		exit(1);
    }
	// Initialize the cache and the semaphores
	init();
    while (1) {
		clientlen = sizeof(clientaddr);
		connfd_ptr = malloc(sizeof(int));
		*connfd_ptr = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE,
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
    	// Assign the job to a new thread
    	if (pthread_create(&tid, NULL, thread, connfd_ptr) != 0) {
			fprintf(stderr, "create thread failed\n");
    		Close(*connfd_ptr);
    		free(connfd_ptr);
		}
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
	char res_buf[MAX_OBJECT_SIZE];
    rio_t client_rio, server_rio;

    Rio_readinitb(&client_rio, client_fd);
    if (!Rio_readlineb(&client_rio, buf, MAXLINE))
        return;
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
	printf("Original uri: %s\n", uri);
    if (strcasecmp(method, "GET")) {
        clienterror(client_fd, method, "501", "Not Implemented",
                    "Proxy does not implement this method");
        return;
    }
	// Search the uri in the cache. If found, return the result directly
	int res_size = check_in_cache(uri, res_buf);
	if (res_size != -1) {
		Rio_writen(client_fd, res_buf, res_size);
		return;
	}

    /* Parse URI from GET request */
	strcpy(port, "80");
    parse_uri(uri, host, path, port);
	create_request(request_to_server, host, path, &client_rio);
	// Connect to the server
	if ((server_fd = Open_clientfd(host, port)) < 0) {
		fprintf(stderr, "Proxy can not connect to the server\n");
		return;
	}

	Rio_readinitb(&server_rio, server_fd);
	Rio_writen(server_fd, request_to_server, strlen(request_to_server));
	int num;
	res_size = 0;
	while ((num = Rio_readlineb(&server_rio, buf, MAXLINE)) > 0) {
		printf("Proxy received %d bytes from the server\n", num);
		Rio_writen(client_fd, buf, num);
		if (res_size + num < MAX_OBJECT_SIZE) {
			strncpy(res_buf + res_size, buf, num);
			res_size += num;
		}
	}
	close(server_fd);
	// If the result is cachable, write it to the cache
	if (res_size < MAX_OBJECT_SIZE)
		write_to_cache(uri, res_buf, res_size);
	return;
}
/* $end doit */

// The thread function
void *thread(void *vargp) {
	int client_fd = *(int *)vargp;
	if (pthread_detach(pthread_self()) != 0) {
		fprintf(stderr, "tid: %lu detach failed\n", (unsigned long)pthread_self());
		Close(client_fd);
		free(vargp);
		return NULL;
	}
	free(vargp);
	doit(client_fd);
	Close(client_fd);
	return NULL;
}

/*
 * parse_uri - parse URI into several components
 * we make sure that the uri is unchanged after parsing
 * since we need to save it into the cache
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
		*temp = ':';
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


// Set up the request needed to send to the server
void create_request(char *request_to_server, char *host, char *path, rio_t *rp) {
	char buf[MAXLINE], additional_hdr[MAXLINE], request[MAXLINE], host_hdr[MAXLINE];
	sprintf(host_hdr, "Host: %s\r\n", host);

	while (Rio_readlineb(rp, buf, MAXLINE) > 0) {
		// End of the request
		if (!strcmp(buf, "\r\n"))
			break;
		// If the host is specified in the header, record it
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

// Initialze the cache and the semaphores
void init() {
	Sem_init(&mutex, 0, 1);
	Sem_init(&w, 0, 1);
	for (int i = 0; i < MAX_NUM_BLOCK; i++) {
		cache[i].uri = NULL;
		cache[i].result = NULL;
		cache[i].timestamp = 0;
		cache[i].uri_size = 0;
		cache[i].res_size = 0;
	}
}

// Search the uri in the cache, if found, copy the result into res_buf and return the size of the result
int check_in_cache(char *uri, char *res_buf) {
	int res_size = -1;
	// Set up the reader-favor model
	P(&mutex);
	readcnt++;
	if (readcnt == 1)
		P(&w);
	V(&mutex);

	for (int i = 0; i < MAX_NUM_BLOCK; i++) {
		// The following lines are for debugging
		//if (cache[i].uri) {
			//printf("Comparing!\n");
			//printf("current uri: %s\n", uri);
			//printf("saved uri: %s\n", cache[i].uri);
		//}
		if (cache[i].uri != NULL && (strcmp(uri, cache[i].uri) == 0)) {
			strncpy(res_buf, cache[i].result, cache[i].res_size);
			res_size = cache[i].res_size;
			// one can add locks here to make it more thread-safe
			cache[i].timestamp = (++current_stamp);
			break;
		}
	}

	P(&mutex);
	readcnt--;
	if (readcnt == 0)
		V(&w);
	V(&mutex);

	return res_size;
}

// Write the uri and the result to the cache
void write_to_cache(char *uri, char *res_buf, int res_size) {
	// Set up the reader-favor model
	P(&w);
	int uri_size = strlen(uri) + 1;
	int evict_index = -1;
	unsigned long stamp = current_stamp;
	// When the cache size is not enough, remove blocks according to (roughly) LRU
	while (uri_size + res_size + current_used_size > MAX_CACHE_SIZE) {
		evict_index = -1;
		stamp = current_stamp;
		for (int i = 0; i < MAX_NUM_BLOCK; i++) {
			if (cache[i].uri && cache[i].timestamp < stamp) {
				evict_index = i;
				stamp = cache[i].timestamp;
			}
		}
		if (evict_index == -1)
			break;
		free(cache[evict_index].uri);
		free(cache[evict_index].result);
		current_used_size -= cache[evict_index].uri_size;
		current_used_size -= cache[evict_index].res_size;
	}
	// Add the uri and the result to the cache
	// If empty block exists, add to it directly
	// If not, remove a block to store the data according to (roughly) LRU
	int empty_index = -1;
	evict_index = -1;
	stamp = current_stamp;
	for (int i = 0; i < MAX_NUM_BLOCK; i++) {
		if (cache[i].uri == NULL) {
			empty_index = i;
			break;
		}
		if (cache[i].timestamp < stamp) {
			evict_index = i;
			stamp = cache[i].timestamp;
		}
	}
	int index;
	if (empty_index != -1) {
		index = empty_index;
	} else {
		index = evict_index;
		free(cache[index].uri);
		free(cache[index].result);
		current_used_size -= cache[index].uri_size;
		current_used_size -= cache[index].res_size;
	}
	cache[index].uri = malloc(uri_size);
	strcpy(cache[index].uri, uri);
	cache[index].uri_size = uri_size;
	cache[index].result = malloc(res_size);
	strncpy(cache[index].result, res_buf, res_size);
	cache[index].res_size = res_size;
	cache[index].timestamp = (++current_stamp);
	current_used_size += uri_size;
	current_used_size += res_size;

	V(&w);
	printf("Saved uri: %s\n",uri);
}

// To make our proxy more robust, we need to handle the prematurely closed reader and writer problem.
// This can be done by modifying the behaviors of the read and the write functions in csapp.c.
// Essentially, the original implementation does not work due to the function unix_error().
// This function will terminate the process when we receive signals like SIGPIPE and this is not expected.
// In these special cases, one should not call unix_error() and instead one should terminate the connection
// to the client and the server.
// We omit the modifications to the csapp.c in this lab.
void sigpipe_handler(int sig) {
	return ;
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