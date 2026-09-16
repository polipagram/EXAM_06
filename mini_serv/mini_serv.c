#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>

char buffer[1024];
int id = 0;
int client_ids[65536];
char *client_buff[65536];
int listen_socket;


void   fatal(void)
{
    write(2, "Fatal error\n", 12);
    exit(1);
}

void    broadcast(fd_set *clientfds, int sender_fd, int maxfd, char *msg, int len)
{
    int fd;
    for(fd = 0; fd <= maxfd; fd++)
    {
        if(FD_ISSET(fd, clientfds) && fd != sender_fd && fd != listen_socket)
            send(fd, msg, len, 0);
    }
}

char  *join_buff(char *buff, char *to_add)
{
    char *n_buff;
    int len;

    if(buff == NULL)
        len = 0;
    else
        len = strlen(buff);
    n_buff = malloc(len + strlen(to_add) + 1);
    if(!n_buff)
        fatal();
    n_buff[0] = '\0';
    if(buff)
        strcat(n_buff, buff);
    strcat(n_buff, to_add);
    free(buff);
    return(n_buff);
}

int main(int ac , char **av)
{
    fd_set    clientfds;
    fd_set    currentfds;
    fd_set    writefds;
    int maxfd;
    int client_fd;
    int bytes;

    if(ac != 2)
    {
        write(2, "Wrong number of arguments\n", 26);
        exit(1);
    }

    listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_socket < 0)
        fatal();
    struct sockaddr_in addr;
    int port = atoi(av[1]);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(2130706433);  //same as 127 | 1 << 24
    addr.sin_port = htons(atoi(av[1])); // same as port << 8 | port >> 8

    if(bind(listen_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        close(listen_socket);
        fatal();
    }

    if(listen(listen_socket, 10) < 0)
    {
        close(listen_socket);
        fatal();
    }

    maxfd = listen_socket;

    FD_ZERO(&clientfds);
    FD_SET(listen_socket, &clientfds);

    while(1)
    {
        currentfds = clientfds;
        writefds = clientfds;

        if(select(maxfd + 1, &currentfds, &writefds, NULL, NULL) < 0)
        {
            close(listen_socket);
            fatal();
        }

        for(int fd = 0; fd <= maxfd; fd++)
        {
            if(!FD_ISSET(fd, &currentfds))
                continue;
            if(fd == listen_socket)
            {
                client_fd = accept(listen_socket, 0, 0);
                if(client_fd >= 0)
                {

                    client_ids[client_fd] = id;
                    client_buff[client_fd] = NULL;

                    FD_SET(client_fd , &clientfds);
                    if(client_fd > maxfd)
                        maxfd = client_fd;
                    sprintf(buffer, "server: client %d just arrived\n", id);
                    broadcast(&writefds, client_fd, maxfd, buffer, strlen(buffer));
                    id++;
                }
            }
            else
            {
                bytes = recv(fd, buffer, sizeof(buffer) -1, 0);
                if(bytes <= 0)
                {
                    sprintf(buffer, "server: client %d just left\n", client_ids[fd]);
                    broadcast(&writefds, fd, maxfd, buffer, strlen(buffer));
                    free(client_buff[fd]);
                    client_buff[fd] = NULL;
                    close(fd);
                    FD_CLR(fd, &clientfds);
                }
                else
                {   
                    buffer[bytes] = '\0';
                    client_buff[fd] = join_buff(client_buff[fd], buffer);

                    char *newline;
                    while((newline = strstr(client_buff[fd], "\n")) != NULL)
                    {
                        int len;
                        char *message;
                        char save;
                        len = newline - client_buff[fd] + 1;
                        message = malloc(len + 1);
                        if(!message)
                            fatal();
                        // the char after newline in the buff
                        save = newline[1];
                        newline[1] = '\0';
                        strcpy(message , client_buff[fd]);
                        newline[1] = save;
                        char prefix[100];
                        sprintf(prefix, "client %d: ", client_ids[fd]);
                        broadcast(&writefds, fd, maxfd, prefix, strlen(prefix));
                        broadcast(&writefds, fd, maxfd, message, strlen(message));
                        free(message);
                        
                        char *remaining = malloc(strlen(client_buff[fd] + len) + 1);
                        if(!remaining)
                            fatal();
                        strcpy(remaining, client_buff[fd] + len);
                        free(client_buff[fd]);
                        client_buff[fd] = remaining;
                    }
                }   
            }
        }
    }
}