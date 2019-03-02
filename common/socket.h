#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <time.h>

#ifdef __cplusplus

#include <string>
namespace cf {

class Socket {
public:
    typedef enum {
        eSocketType_None = 0,
        eSocketType_TCP,
        eSocketType_UDP,
    }SOCKETTYPE;

    Socket();
    Socket(SOCKETTYPE eSocketType);
    Socket(const Socket& o);
    virtual ~Socket();
    Socket& operator=(const Socket& o);

    bool isValid();
    void reset();

    // true: success.
    // false: fail.
    bool create(SOCKETTYPE socketType = eSocketType_None);
    bool setRemoteIP(const std::string& ipaddr);
    bool setHostIP(const std::string& ipaddr);
    bool enableBroadcast();
    bool listen();
    bool bind(int port = -1);
    bool connect(const std::string& ipaddr = "", int port = -1);
    Socket accept();
    void accept(Socket* s);

    int fd();
    std::string getRemoteIP();
    std::string getHostIP();
    int getRemotePort();
    int getHostPort();
    void setRemotePort(int port);
    void setHostPort(int port);

    int recv(void * buf, size_t len);
    int send(const void * buf, size_t len);
    int recvFrom(char * ipaddr, int& port, void * buf, size_t len);
    int sendTo(const std::string& ipaddr, int port, const void * buf, size_t len);

    const char * lastErrorMessage();

public:
    virtual void onReadData(const void * buf, int len) {}
    virtual void onDisconnected(void) {}


private:
    int mSock;
    SOCKETTYPE mSockType;
    sockaddr_in mRemote;
    sockaddr_in mHost;
    bool mListening;
};

} // namespace cf 

#endif // ifdef __cplusplus

#ifdef __cplusplus
extern "C" {
#endif

typedef void * socket_t;
#define SOCKET_T_INITIALIZER ((void*)0)

socket_t socket_create_tcp();
void socket_close(socket_t* socket);

int socket_bind_and_listen(socket_t socket, int port);
int socket_connect(socket_t socket, const char * ipaddr, int port);
socket_t socket_accept(socket_t socket);
int socket_fd(socket_t socket);

int socket_set_remote_ip(socket_t socket, const char * ipaddr);
void socket_set_remote_port(socket_t socket, int port);
char * socket_get_remote_ip(socket_t socket, char * buf, int buflen);
int socket_get_remote_port(socket_t socket);

int socket_set_host_ip(socket_t socket, const char * ipaddr);
char * socket_get_host_ip(socket_t socket, char * buf, int buflen);
int socket_get_host_port(socket_t socket);
void socket_set_host_port(socket_t socket, int port);

int socket_recv(socket_t socket, void* buf, size_t len);
int socket_send(socket_t socket, const void * buf, size_t len);


#ifdef __cplusplus
}
#endif












