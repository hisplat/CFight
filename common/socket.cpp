
#include "socket.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <string>

#include "logging.h"

namespace cf {

Socket::Socket()
    : mSock(-1)
    , mSockType(eSocketType_TCP)
    , mListening(false)
{
    mHost.sin_family = AF_INET;
    mHost.sin_addr.s_addr= INADDR_ANY;
    mHost.sin_port = 0;

    mRemote.sin_family = AF_INET;
    mRemote.sin_addr.s_addr = INADDR_ANY;
    mRemote.sin_port = 0;
}

Socket::Socket(SOCKETTYPE eSocketType)
    : mSock(-1)
    , mSockType(eSocketType)
    , mListening(false)
{
    mHost.sin_family = AF_INET;
    mHost.sin_addr.s_addr = INADDR_ANY;
    mHost.sin_port = 0;
    mRemote.sin_family = AF_INET;
    mRemote.sin_addr.s_addr = INADDR_ANY;
    mRemote.sin_port = 0;
}

Socket::Socket(const Socket& o)
{
    mSock = o.mSock;
    mSockType = o.mSockType;
    memcpy(&mRemote, &o.mRemote, sizeof(mRemote));
    memcpy(&mHost, &o.mHost, sizeof(mHost));
    mListening = o.mListening;
}

Socket& Socket::operator=(const Socket& o)
{
    mSock = o.mSock;
    mSockType = o.mSockType;
    memcpy(&mRemote, &o.mRemote, sizeof(mRemote));
    memcpy(&mHost, &o.mHost, sizeof(mHost));
    mListening = o.mListening;
    return *this;
}

Socket::~Socket()
{
}

bool Socket::create(SOCKETTYPE socketType)
{
    SOCKETTYPE ost = mSockType;
    if (socketType != eSocketType_None) {
        ost = socketType;
    }
    reset();
    mSockType = ost;
    if (mSockType == eSocketType_TCP) {
        mSock = socket(PF_INET, SOCK_STREAM, 0);
    } else if (mSockType == eSocketType_UDP) {
        mSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }
    if (mSock == -1) {
        return false;
    }
    return true;
}

int Socket::fd()
{
    return mSock;
}

std::string Socket::getRemoteIP()
{
    char ip[1024] = {0};
    inet_ntop(AF_INET, &mRemote.sin_addr, ip, sizeof(ip));
    return ip;
    // return static_cast<const char *>(inet_ntoa(mRemote.sin_addr));
}

bool Socket::setRemoteIP(const std::string& ipaddr)
{
    // mRemote.sin_addr.s_addr = inet_addr(ipaddr);
    if (inet_aton(ipaddr.c_str(), &mRemote.sin_addr) != 0) {
        return true;
    }
    return false;
}

int Socket::getRemotePort()
{
    return static_cast<int>(ntohs(mRemote.sin_port));
}

void Socket::setRemotePort(int port)
{
    mRemote.sin_port = htons(port);
}

std::string Socket::getHostIP()
{
    char ip[1024] = {0};
    inet_ntop(AF_INET, &mHost.sin_addr, ip, sizeof(ip));
    return ip;
    // return static_cast<const char *>(inet_ntoa(mHost.sin_addr));
}

bool Socket::setHostIP(const std::string& ipaddr)
{
    if (inet_aton(ipaddr.c_str(), &mHost.sin_addr) != 0) {
        return true;
    }
    return false;
}

int Socket::getHostPort()
{
    return static_cast<int>(ntohs(mHost.sin_port));
}

void Socket::setHostPort(int port)
{
    mHost.sin_port = htons(port);
}

bool Socket::enableBroadcast()
{
    int onoff = 1;
    if (0 != setsockopt(mSock, SOL_SOCKET, SO_BROADCAST, (char *)&onoff, sizeof(int))) {
        return false;
    }
    return true;
}

bool Socket::listen()
{
    return (::listen(mSock, 10) == 0);
}

bool Socket::bind(int port)
{
    if (port >= 0) {
        setHostPort(port);
    }
    int op = 1;
    int ret = setsockopt(mSock, SOL_SOCKET, SO_REUSEADDR, (char *)&op, sizeof(op));
    if(ret != 0) {
        perror("setsockopt.SO_REUSEADDR");
    }
    ret = ::bind(mSock, (struct sockaddr *)&mHost, sizeof(mHost));
    return (ret == 0);
}

bool Socket::connect(const std::string& ipaddr, int port)
{
    if (ipaddr != "") {
        setRemoteIP(ipaddr);
    }
    if (port != -1) {
        setRemotePort(port);
    }
    int ret = ::connect(mSock, (struct sockaddr *)&mRemote, sizeof(mRemote));
    return (ret == 0);
}

Socket Socket::accept()
{
    socklen_t len;
    int fd;
    len = sizeof(mRemote);
    fd = ::accept(mSock, (struct sockaddr *)&mRemote, &len);

    Socket sock;
    sock.reset();
    sock.mSock = fd;
    sock.setRemoteIP(getRemoteIP());
    sock.setRemotePort(getRemotePort());
    return sock;
}

void Socket::accept(Socket* s)
{
    socklen_t len = sizeof(mRemote);
    int fd = ::accept(mSock, (struct sockaddr *)&mRemote, &len);
    s->reset();
    s->mSock = fd;
    s->setRemoteIP(getRemoteIP());
    s->setRemotePort(getRemotePort());
}


int Socket::recv(void * buf, size_t len)
{
    socklen_t sl = sizeof(mRemote);
    int ret;

    if (mSockType == eSocketType_UDP) {
        ret = ::recvfrom(mSock, (char *)buf, len, 0, (struct sockaddr *)&mRemote, &sl);
    } else {
        ret = ::read(mSock, (char *)buf, len);
    }
    if (ret <= 0) {
        onDisconnected();
    } else {
        onReadData(buf, ret);
    }
    return ret;
}

int Socket::send(const void * buf, size_t len)
{
    socklen_t sl = sizeof(mRemote);
    if (mSockType == eSocketType_UDP) {
        return ::sendto(mSock, (const char *)buf, len, 0, (struct sockaddr *)&mRemote, sl);
    }  else {
        return ::write(mSock, (const char *)buf, len);
    }
}

int Socket::recvFrom(char * ipaddr, int& port, void * buf, size_t len)
{
    int             ret;
    ret = recv(buf, len);
    strcpy(ipaddr, getRemoteIP().c_str());
    port = getRemotePort();
    return ret;
}

int Socket::sendTo(const std::string& ipaddr, int port, const void * buf, size_t len)
{
    setRemoteIP(ipaddr);
    setRemotePort(port);
    return send(buf, len);
}

void Socket::reset()
{
    if (mSock != -1) {
        close(mSock);
    }
    mSock = -1;
    mListening = false;
    mSockType = eSocketType_TCP;
    mHost.sin_family = AF_INET;
    mHost.sin_addr.s_addr = INADDR_ANY;
    mHost.sin_port = 0;
    mRemote.sin_family = AF_INET;
    mRemote.sin_addr.s_addr = INADDR_ANY;
    mRemote.sin_port = 0;
}

bool Socket::isValid(void)
{
    return (mSock != -1);
}

const char * Socket::lastErrorMessage()
{
    return strerror(errno);
}


} // namespace cf

static cf::Socket * convert_to_cf_socket(socket_t socket)
{
    // TODO. check socket class.
    return (cf::Socket*)socket;
}

static socket_t convert_to_socket_t(cf::Socket* s)
{
    return (socket_t)s;
}

socket_t socket_create_tcp()
{
    cf::Socket* s = new cf::Socket();
    s->create(cf::Socket::eSocketType_TCP);
    return (socket_t)s;
}

void socket_close(socket_t* socket)
{
    cf::Socket * s = convert_to_cf_socket(*socket);
    delete s;
    *socket = NULL;
}

int socket_bind_and_listen(socket_t socket, int port)
{
    cf::Socket * s = convert_to_cf_socket(socket);
    s->bind(port);
    errno = 0;
    return (s->listen() ? 0 : -errno);
}

int socket_connect(socket_t socket, const char * ipaddr, int port)
{
    cf::Socket * s = convert_to_cf_socket(socket);
    errno = 0;
    return (s->connect(ipaddr, port) ? 0 : -errno);
}

socket_t socket_accept(socket_t socket)
{
    socket_t ns = socket_create_tcp();

    cf::Socket * s = convert_to_cf_socket(socket);
    cf::Socket * nns = convert_to_cf_socket(ns);
    s->accept(nns);
    return convert_to_socket_t(nns);
}

int socket_fd(socket_t socket)
{
    cf::Socket * s = convert_to_cf_socket(socket);
    return s->fd();
}

int socket_set_remote_ip(socket_t socket, const char * ipaddr)
{
    cf::Socket * s = convert_to_cf_socket(socket);
    errno = 0;
    return (s->setRemoteIP(ipaddr) ? 0 : -errno);
}

void socket_set_remote_port(socket_t socket, int port)
{
    cf::Socket * s = convert_to_cf_socket(socket);
    s->setRemotePort(port);
}

char * socket_get_remote_ip(socket_t socket, char * buf, int buflen)
{
    cf::Socket * s = convert_to_cf_socket(socket);
    std::string ip = s->getRemoteIP();
    int l = ip.length();
    if (l > buflen) {
        l = buflen;
    }
    snprintf(buf, l, "%s", ip.c_str());
    return buf;
}

int socket_get_remote_port(socket_t socket)
{
    cf::Socket * s = convert_to_cf_socket(socket);
    return s->getRemotePort();
}

int socket_set_host_ip(socket_t socket, const char * ipaddr)
{
    cf::Socket * s = convert_to_cf_socket(socket);
    errno = 0;
    return (s->setHostIP(ipaddr) ? 0 : -errno);
}


char * socket_get_host_ip(socket_t socket, char * buf, int buflen)
{
    cf::Socket * s = convert_to_cf_socket(socket);
    std::string ip = s->getHostIP();
    int l = ip.length();
    if (l > buflen) {
        l = buflen;
    }
    snprintf(buf, l, "%s", ip.c_str());
    return buf;
}

int socket_get_host_port(socket_t socket)
{
    cf::Socket * s = convert_to_cf_socket(socket);
    return s->getHostPort();
}

void socket_set_host_port(socket_t socket, int port)
{
    cf::Socket * s = convert_to_cf_socket(socket);
    s->setHostPort(port);
}

int socket_recv(socket_t socket, void* buf, size_t len)
{
    cf::Socket * s = convert_to_cf_socket(socket);
    return s->recv(buf, len);
}

int socket_send(socket_t socket, const void * buf, size_t len)
{
    cf::Socket * s = convert_to_cf_socket(socket);
    return s->send(buf, len);
}

