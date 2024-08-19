#include "csapp.h"
int open_clientfd(char *hostname, char *port) {
    int clientfd;
    struct addrinfo hints, *listp, *p;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrrinfo));
    hints.ai_socktype = SOCK_STREAM; // TCP Socket
    hints.ai_flags = AI_NUMBERICSERV;
    hints.ai_flags |= AI_ADDRCONFIG; // 시스템의 현재 네트워크 구성에 따라 주소 반환 요청
    Getaddrinfo(hostname, port, &hints, &listp);

    /* Walk the list for one that we can successfully connect to */
    for (p = listp; p; p = p->ai_next) {
        /* Create a socket descriptor */
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) // 소켓 생성 실패
            continue;
        
        /* Connect to the server */
        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1) // 연결 성공
            break;
        Close(clientfd); // 연결 실패
    }

    Freeaddrinfo(listp);
    if (!p) // 모든 연결 실패
        return -1;
    else
        return clientfd;
}