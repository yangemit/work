#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <netinet/in.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

int get_host_from_url(const char *url, char *hostbuf, int hostsize);
int get_sockaddr(struct sockaddr_in *serverAddr, const char *url, const char *ip, const int port);
int create_socket(struct sockaddr_in *addr);
int release_socket(int sockfd);
int ComSockRecv(int sockfd, void *buf, ssize_t len);
int ComSockSend(int sockfd, void *buf, ssize_t len);
int setNonBlocking(int fd);
int setSndRcvTimeOut(int fd, int timeOutMs);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
