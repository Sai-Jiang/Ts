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

#define N 	(20000)
//#define USETCP	(1)
#define LEN	(1024)

long getts() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long ts = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    return ts;
}

ssize_t nrecv(int sock, void *buf, size_t len, int flags) {
#ifdef USETCP
	size_t nr;
	size_t nleft = len;
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
#else
	return recv(sock, buf, len, flags);
#endif
}

typedef struct {
	long ts;
	char buf[LEN - sizeof(long)];
} DataWrapper;

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Not Enough Parameter\n");
        return -1;
    } else if (memcmp(argv[1], "c", 1) == 0) {
#ifdef USETCP
        // Init Socket Begins
        int sock = socket(PF_INET, SOCK_STREAM, 0);
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        inet_pton(PF_INET, "127.0.0.1", &addr.sin_addr);
        addr.sin_family = AF_INET;
        addr.sin_port = htons(7777);
        connect(sock, (struct sockaddr *) &addr, sizeof(addr));
        int on = 1;
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
	// Init Socket Ends
#else
	int sock = socket(PF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        inet_pton(PF_INET, "127.0.0.1", &addr.sin_addr);
        addr.sin_family = AF_INET;
        addr.sin_port = htons(7777);
        connect(sock, (struct sockaddr *) &addr, sizeof(addr));
#endif
	DataWrapper dw;

        for (int i = 0; i < N; i++) {
	    dw.ts = getts();	
            send(sock, &dw, sizeof(dw), 0);

            // 1000KBps => 1KB/ms => 1 packet / 1000us
            usleep(1000);
        }
    } else if (memcmp(argv[1], "s", 1) == 0) {
#ifdef USETCP
	// Init Socket Begins
        int listensock = socket(PF_INET, SOCK_STREAM, 0);
        int opt = 1;
        setsockopt(listensock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htons(INADDR_ANY);
        addr.sin_port = htons(7777);
        if (bind(listensock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            printf("tcpsock Bind Failed\n");
            return -1;
        }
        listen(listensock, 128);
        int sock = accept(listensock, NULL, NULL);
        int on = 1;
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
	// Init Socket Ends
#else
	int sock = socket(PF_INET, SOCK_DGRAM, 0);
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htons(INADDR_ANY);
        addr.sin_port = htons(7777);
        if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            printf("tcpsock Bind Failed\n");
            return -1;
        }

	struct timeval timeout = {2, 0};
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (void *)&timeout, sizeof(struct timeval));
#endif
	DataWrapper dw;

        for (int i = 0; i < N; i++) {
	    int nrcv = nrecv(sock, &dw, sizeof(dw), 0);
	    if (nrcv != sizeof(dw)) {
	    	fprintf(stderr, "nrcv: %d\n", nrcv);
		return -1;
	    }
	    //assert(nrcv == sizeof(dw));
	    printf("%ld\n", getts() - dw.ts);
	}
    } else {
        printf("Error Parameter\n");
        return -1;
    }
}

