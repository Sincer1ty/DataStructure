#include "csapp.h"
int open_listenfd(char *port) {
    struct addrinfo hints, *listp, *p;
    int listenfd, optval=1;

    /* Get a list of potential server addresses*/
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM; // TCP Socket
    // AI_PASSIVE : 특정 IP 주소가 아닌 모든 가용한 네트워크 인터페이스에서 들어오는 연결을 받아들이고자 할 때 사용
    // AI_ADDRCONFIG : 시스템의 현재 네트워크 구성에 따라 주소 반환 요청
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_flags |= AI_NUMERICSERV;
    Getaddrinfo(NULL, port, &hints, &listp);

    /* Walk the list for one that we can bind to */
    for (p = listp; p; p = p->ai_next) {
        /* Create a socket descriptor */
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;

        /* Eliminates "Address already in use" error from bind */   
        Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 
                    (const void *)&optval, sizeof(int));

        /* Bind the descriptor to the address */
        // 서버는 고정된 주소(IP:포트)에서 클라이언트의 연결을 기다려야 함
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break; // 바인딩 성공
        Close(listenfd);
    }

    /* Clean up */
    Freeaddrinfo(listp);
    if (!p) // 작동하는 주소 없음
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    // listen : 클라이언트의 연결 요청 대기
    if (listen(listenfd, LISTENQ) < 0) { // 오류 발생
        Close(listenfd);
        return -1;
    }
    return listenfd;
}