#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>

#include "network.h"

int get_host_from_url(const char *url, char *hostbuf, int hostsize)
{
    int ret = 0, bfound = 0;
    struct addrinfo hints;
    struct addrinfo *result, *result_pointer;

    /* obtaining address matching host */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
    hints.ai_protocol = 0;  /* any protocol */

    ret = getaddrinfo(url, NULL, &hints, &result);
    if (ret != 0) {
        printf("no valid host to url:%s\n", gai_strerror(ret));
        return -1;
    }

    /* traverse the returned list and output the ip addresses */
    for (result_pointer = result; result_pointer != NULL; result_pointer = result_pointer->ai_next) {
        ret = getnameinfo(result_pointer->ai_addr, result_pointer->ai_addrlen, hostbuf, hostsize, NULL, 0, NI_NUMERICHOST);
        if (ret == 0) {
            bfound = 1;
            break;
        }
    }
    freeaddrinfo(result);

    if (!bfound) {
        printf("can't find valid server ip\n");
        return -1;
    }

    return 0;
}

int get_sockaddr(struct sockaddr_in *serverAddr, const char *url, const char *ip, const int port)
{
    int ret = 0;
    char hostbuf[NI_MAXHOST];

    if (ip && (strlen(ip) > 0)) {
        strncpy(hostbuf, ip, sizeof(hostbuf));
    } else {
        ret = get_host_from_url(url, hostbuf, sizeof(hostbuf));
        if (ret < 0) {
            printf("get_host_from_url failed\n");
            return -1;
        }
    }

	serverAddr->sin_family = AF_INET;
	serverAddr->sin_port = htons(port);
    serverAddr->sin_addr.s_addr = inet_addr(hostbuf);
	bzero(serverAddr->sin_zero, 8);

    return 0;
}

static int connect_retry(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	int msec = 0;

	for (msec = 8; msec <= 64; msec <<= 1) {
		if (connect(sockfd, addr, addrlen) == 0) {
			printf("connect successfully,pid=%d\n", getpid());
			return 0;
		}
		usleep(msec * 1000);
	}

	printf("connect to server failed:%s,pid=%d\n", strerror(errno), getpid());

	return -1;
}

int create_socket(struct sockaddr_in *addr)
{
	int ret;
	int sockfd = -1;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		printf("create socket failed:%s, pid=%d!\n", strerror(errno), getpid());
        goto err_open_socket;
	}

	ret = setSndRcvTimeOut(sockfd, 5000);
	if (ret < 0) {
		printf("setSndRcvTimeOut to 5s failed\n");
		goto err_setSndRcvTimeOut;
	}

	ret = connect_retry(sockfd, (const struct sockaddr *)addr, sizeof(struct sockaddr_in));
	if (ret < 0) {
		printf("connect to server failed!\n");
        goto err_connect_retry;
	}

	return sockfd;

err_connect_retry:
err_setSndRcvTimeOut:
    close(sockfd);
err_open_socket:
    return -1;
}

int release_socket(int sockfd)
{
	close(sockfd);

	return 0;
}

int ComSockRecv(int sockfd, void *buf, ssize_t len)
{
	ssize_t recvSize = 0, recvTotalSize = 0;
	int retryTimes = (len + 63) / 64;

	if ((sockfd < 0) || (buf == NULL) || (len == 0)) {
		printf("%s:invalid param,sockfd=%d,buf=%p,len=%d\n", __func__, sockfd, buf, (int)len);
		goto err_invalid_recv_param;
	}

	while ((len - recvTotalSize > 0) && (retryTimes > 0)) {
		recvSize = recv(sockfd, buf, len - recvTotalSize, 0);
		if (recvSize < 0) {
			if ((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINTR)) {
				if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
					retryTimes--;
				}
				continue;
			} else {
				printf("recv sockfd(%d) failed:errno:%x,%s,pid=%d\n", sockfd, errno, strerror(errno), getpid());
				goto err_recv_sockfd_error;
			}
		} else if (recvSize == 0) {
			printf("sockfd(%d) recvTotalSize=%ld bytes has closed:%s,pid=%d\n", sockfd, (long)recvTotalSize, strerror(errno), getpid());
			goto err_recv_sockfd_closed;
		}
		recvTotalSize += recvSize;
	}

	return recvTotalSize;

err_recv_sockfd_closed:
	return 0;
err_recv_sockfd_error:
err_invalid_recv_param:
	return -1;
}

int ComSockSend(int sockfd, void *buf, ssize_t len)
{
	ssize_t sendSize = 0, sendTotalSize = 0;
	int retryTimes = (len + 63) / 64;

	if ((sockfd < 0) || (buf == NULL) || (len == 0)) {
		printf("%s:invalid param,sockfd=%d,buf=%p,len=%d\n", __func__, sockfd, buf, (int)len);
		goto err_invalid_send_param;
	}

	while ((len - sendTotalSize > 0) && (retryTimes > 0)) {
		sendSize = send(sockfd, buf, len - sendTotalSize, 0);
		if (sendSize < 0) {
			if ((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINTR)) {
				if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
					retryTimes--;
				}
				continue;
			} else {
				printf("send sockfd(%d) sendTotalSize=%ld bytes failed:%s,pid=%d\n",
						sockfd, (long)sendTotalSize, strerror(errno), getpid());
				goto err_send_sockfd_error;
			}
		}
		sendTotalSize += sendSize;
	}

	return sendTotalSize;

err_send_sockfd_error:
err_invalid_send_param:
	return -1;
}

int setNonBlocking(int fd)
{
	int flags;

	flags = fcntl(fd, F_GETFL);
	if (flags < 0) {
		printf("fcntl(%d, GETFL) failed: %s\n", fd, strerror(errno));
		return -1;
	}

	flags = flags | O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0) {
		printf("fcntl(%d, SETFL, opts) faild: %s", fd, strerror(errno));
		return -1;
	}

	return 0;
}

int setSndRcvTimeOut(int fd, int timeOutMs)
{
	struct timeval tv = {timeOutMs / 1000, (timeOutMs % 1000) * 1000};

	if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv)) < 0) {
		printf("set SO_SNDTIMEO to timeOutMs=%d failed!\n", timeOutMs);
		goto err_set_sndtimeo;
	}

	if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0) {
		printf("set SO_SNDTIMEO to timeOutMs=%d failed!\n", timeOutMs);
		goto err_set_rcvtimeo;
	}

	return 0;

err_set_rcvtimeo:
err_set_sndtimeo:
	return -1;
}
