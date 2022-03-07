#include "ib.hpp"
#include "common.hpp"

#include <cstdlib>
#include <infiniband/verbs.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>

bool serverConnect(const char *port) {
    addrinfo hints;
    addrinfo *result;

    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, port, &hints, &result)) {
        logErr("Failed to getaddrinfo");
        return false;
    }

    int sock = -1;
    for (auto rp = result; rp != nullptr; rp = rp->ai_next) {
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock > 0) {
            if (!bind(sock, rp->ai_addr, rp->ai_addrlen)) {
                break;
            }

            close(sock);
            sock = -1;
        }
    }

    freeaddrinfo(result);
    if (sock == -1) {
        logErr("Cannot create or bind socket.");
        return false;
    }

    listen(sock, 5);

    sockaddr_in peerAddr;
    socklen_t peerAddrLen;
    int peerSock = accept(sock, (sockaddr *)&peerAddr, &peerAddrLen);
    if (peerSock < 0) {
        logErr("Failed to create peer socket.");
        return false;
    }

    return true;
}

bool clientConnect(const char *serverName, const char *port) {
    addrinfo hints;
    addrinfo *result;

    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;

    if (getaddrinfo(serverName, port, &hints, &result)) {
        logErr("Failed to getaddrinfo");
        return false;
    }

    int sock = -1;
    for (auto rp = result; rp != nullptr; rp = rp->ai_next) {
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock > 0) {
            if (!connect(sock, rp->ai_addr, rp->ai_addrlen)) {
                break;
            }

            close(sock);
            sock = -1;
        }
    }

    freeaddrinfo(result);
    if (sock == -1) {
        logErr("Cannot connect");
        return false;
    }
    return true;
}

bool setupIb() {
    int numDevice;
    ibv_device **devList = ibv_get_device_list(&numDevice);

    if (numDevice <= 0) {
        logErr("Didn't find IB devices.");
        return false;
    }

    // Open the first device.
    ctx.context = ibv_open_device(*devList);
    if (!ctx.context) {
        logErr("Couldn't get IB context.");
        return false;
    }

    // Allocate protection domain;
    ctx.pd = ibv_alloc_pd(ctx.context);
    if (!ctx.pd) {
        logErr("Failed to allocate protection domain.");
        return false;
    }

    // The registered memory buffer doesn't have to be page-aligned. But NCCL
    // say: This needs to be allocated on separate pages as those pages will be
    // marked DONTFORK and if they are shared, that could cause a crash in a
    // child process
    ctx.buffer = (char *)aligned_alloc(4096, config.blockSize * config.txDepth);

    ctx.mr = ibv_reg_mr(ctx.pd, ctx.buffer, config.blockSize * config.txDepth,
                        IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_READ |
                            IBV_ACCESS_LOCAL_WRITE);
    if (!ctx.mr) {
        logErr("Failed to register memory region.");
        return false;
    }

    ctx.cq = ibv_create_cq(ctx.context, config.txDepth, nullptr, nullptr, 0);

    return true;
}