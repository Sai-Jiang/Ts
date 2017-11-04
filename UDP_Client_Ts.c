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
#include <stdbool.h>
#include <stdlib.h>

#define LEN	(1024)

#define min(a,b) \
({ __typeof__ (a) _a = (a); \
__typeof__ (b) _b = (b); \
_a < _b ? _a : _b; })

#define max(a,b) \
({ __typeof__ (a) _a = (a); \
__typeof__ (b) _b = (b); \
_a > _b ? _a : _b; })

long GetTS() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long ts = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    return ts;
}

typedef struct {
    long ts;
    uint32_t CurCapactiy;
    uint32_t MaxCapacity;
    double LimitedRate;
} TokenBucket;

void TokenBucketInit(TokenBucket *tb, double rate)
{
    tb->ts = GetTS();
    tb->CurCapactiy = 0;
    tb->MaxCapacity = 4096;
    tb->LimitedRate = rate; // Unit: Byte/ms
}

void PutToken(TokenBucket *tb)
{
    if (tb->CurCapactiy >= tb->MaxCapacity) return;
    long Now = GetTS();
    assert(Now >= tb->ts);
    uint32_t reload = (uint32_t)((Now - tb->ts) * tb->LimitedRate);
    assert(reload >= 0);
    if (reload > 0) {
        tb->ts = Now;
        tb->CurCapactiy = min(tb->CurCapactiy + reload, tb->MaxCapacity);
    }
}

bool GetToken(TokenBucket *tb, size_t need)
{
    PutToken(tb);
    
    bool rval = false;
    
    if (tb->CurCapactiy >= need) {
        tb->CurCapactiy -= need;
        rval = true;
    }
    
    return rval;
}

typedef struct {
    long seq;
	long ts;
	char buf[LEN - sizeof(long) - sizeof(long)];
} DataWrapper;

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: cmd N rate\n");
        return -1;
    }
    
    int N = atoi(argv[1]);
    int rate = atoi(argv[2]);
    
    int sock = socket(PF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    inet_pton(PF_INET, "127.0.0.1", &addr.sin_addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(7777);
    connect(sock, (struct sockaddr *) &addr, sizeof(addr));
    
    TokenBucket tb;
    TokenBucketInit(&tb, rate);

    DataWrapper dw;
    
    for (int i = 0; i < N; ) {
        if (GetToken(&tb, sizeof(dw)) == false)
            continue;
        
        dw.seq = i++;
        dw.ts = GetTS();
        send(sock, &dw, sizeof(dw), 0);
    }
}

