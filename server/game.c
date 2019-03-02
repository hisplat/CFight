
#include "game.h"

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "logging.h"
#include "client.h"


void read_client_data(client_t * client)
{
    char buffer[1024];
    int ret = socket_recv(client->socket, buffer, sizeof(buffer));
    cf_debug("%d bytes read.", ret);
    if (ret <= 0) {
        char ip[100];
        cf_log("[%s:%d] disconnected.\n", socket_get_remote_ip(client->socket, ip, sizeof(ip)), socket_get_remote_port(client->socket));
        client_destroy(client);
    }
}

int main_loop(int port)
{
    client_init();

    socket_t main_socket = socket_create_tcp();
    int mainfd = socket_fd(main_socket);

    int ret = socket_bind_and_listen(main_socket, port);
    if (ret != 0) {
        cf_error("bind to port %d failed: %d, %s\n", port, -ret, strerror(-ret));
        return -1;
    }
    cf_log("listening on port %d\n", port);

    while (1) {
        fd_set  rfds;
        struct timeval tv;
        client_t * client;

        FD_ZERO(&rfds);
        FD_SET(mainfd, &rfds);
        int maxfd = mainfd;

        client_reset_iterator();
        while ((client = client_next()) != NULL) {
            int fd = socket_fd(client->socket);
            FD_SET(fd, &rfds);
            if (fd > maxfd) {
                maxfd = fd;
            }
        }

        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int ret = select(maxfd + 1, &rfds, NULL, NULL, &tv);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            cf_error("will exit server.");
            break;
        } else if (ret == 0) {
            continue;
        } else {
            client_reset_iterator();
            while ((client = client_next()) != NULL) {
                int fd = socket_fd(client->socket);
                if (FD_ISSET(fd, &rfds)) {
                    read_client_data(client);
                }
            }

            // accept new connection.
            if (FD_ISSET(mainfd, &rfds)) {
                socket_t s = socket_accept(main_socket);
                client_t * c = client_create();
                c->socket = s;

                char ip[100];
                cf_log("[%s:%d] connected.", socket_get_remote_ip(s, ip, sizeof(ip)), socket_get_remote_port(s));
            }
        }
    }
    return 0;
}

