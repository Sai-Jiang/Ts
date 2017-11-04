#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

#define LEN	(1024)

long getts() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long ts = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    return ts;
}

ssize_t nrecv(int sock, void *buf, size_t len, int flags) {
    ssize_t nr;
    ssize_t nleft = len;
    void *ptr = buf;
    
    while (nleft > 0) {
        if ((nr = recv(sock, ptr, nleft, flags)) < 0) {
            if (errno == EINTR)
                nr = 0;
            else
                return -1;
        } else if (nr == 0)
            break;
        
        nleft -= nr;
        ptr += nr;
    }
    
    return (len - nleft);
}

typedef struct {
	long ts;
	char buf[LEN - sizeof(long)];
} DataWrapper;

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: cmd N\n");
        return -1;
    }
    
    int N = atoi(argv[1]);
    
    int listensock = socket(PF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listensock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htons(INADDR_ANY);
    addr.sin_port = htons(7777);
    assert(bind(listensock, (struct sockaddr *)&addr, sizeof(addr)) == 0);
    listen(listensock, 128);
    int sock = accept(listensock, NULL, NULL);
    int on = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));

    DataWrapper dw;

    for (int i = 0; i < N; i++) {
        int nrcv = nrecv(sock, &dw, sizeof(dw), 0);
        assert(nrcv == sizeof(dw));
        printf("%ld\n", getts() - dw.ts);
    }
}

