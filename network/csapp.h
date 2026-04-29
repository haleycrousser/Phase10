#pragma once //only include this file


#ifdef _WIN32 //MinGW/MSVC automatically defines for windows
  #ifndef WIN32_LEAN_AND_MEAN //slims down windows header file
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <winsock2.h> //networking library - sockets
  #include <ws2tcpip.h> // getaddrinfo etc
  typedef SOCKET socket_t; //make a nickname so the rest of code doesn't care if it's windows
  #define INVALID_SOCK INVALID_SOCKET
#else
  //to use in case we arent on windows.
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <unistd.h>
  typedef int socket_t;
  #define INVALID_SOCK (-1)
#endif

//inclusions
#include <cstring>
#include <cstdio>
#include <cstdlib>

#define RIO_BUFSIZE 8192 //internal read buffer

typedef struct {
    socket_t  rio_fd; //which socket we're reading from
    int       rio_cnt; //how many unread bytes
    char     *rio_bufptr; //unread byte
    char      rio_buf[RIO_BUFSIZE]; //the actual buffer (8KB)
} rio_t;

//zeros everything out when called after connecting
void rio_readinitb(rio_t *rp, socket_t fd);

//read one line from the socket stopping at \n
//returns how many bytes read
//main function to recieve message
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);

//send exactly n bytes to the socket
ssize_t rio_writen(socket_t fd, const void *usrbuf, size_t n);

//open a socket, bind to a port number and start listening for incoming connections
socket_t open_listenfd(int port);

//connect to a server ay hostname:port, return connected socket
socket_t open_clientfd(const char *hostname, int port);

//close socket
void close_socket(socket_t fd);

//windows requires you explicitly start and stop the winsock system
void init_winsock();
void cleanup_winsock();