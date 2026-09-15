#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/select.h>
#include <stdio.h>

int listen_socket;

void	broadcat(fd_set *clientfds, int sender_fd, int maxfd, char *msg, int len)
{
	int	fd;

	for (fd = 0; fd <= maxfd; fd++)
	{
		if (FD_ISSET(fd, clientfds) && fd != sender_fd && fd != listen_socket)
			send(fd, msg, len, 0);
	}
}

int main(int ac, char **av)
{
    fd_set				clientfds;
    fd_set				currentfds;
    fd_set              writefds;
    int maxfd;
    int bytes;
    char buffer[1024];
    int	client_ids[1024];
    int id = 0;
    char	client_buff[1024][1024];
    int		client_len[1024];

    if (ac != 2)
    {
        write(2, "Wrong number of arguments\n", 26);
        exit(1);
    }

    listen_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (listen_socket < 0)
    {
        write(2, "Fatal error\n", 12);
        exit(1);
    }

    struct sockaddr_in addr;

    bzero(&addr, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(2130706433);
    addr.sin_port = htons(atoi(av[1]));

	if (bind(listen_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		close(listen_socket);
		write(2, "Fatal error\n", 12);
		exit(1);
	}

    if (listen(listen_socket, 10) < 0)
    {
	    close(listen_socket);
	    write(2, "Fatal error\n", 12);
	    exit(1);
    }
    maxfd = listen_socket;
    FD_ZERO(&clientfds);
	FD_SET(listen_socket, &clientfds);
    int client_fd;
    while (1)
    {
	    currentfds = clientfds;
        writefds = clientfds;

	    if (select(maxfd + 1, &currentfds, &writefds, NULL, NULL) < 0)
	    {
		    close(listen_socket);
		    write(2, "Fatal error\n", 12);
		    exit(1);
	    }

	    for (int fd = 0; fd <= maxfd; fd++)
	    {
		    if (!FD_ISSET(fd, &currentfds))
			    continue;

            if (fd == listen_socket)
            {
                client_fd = accept(listen_socket, 0, 0);
                if (client_fd < 0)
                {
                    close(listen_socket);
                    write(2, "Fatal error\n", 12);
                    exit(1);
                }
                client_ids[client_fd] = id;
                client_len[client_fd] = 0;

                
                FD_SET(client_fd, &clientfds);
                
                if (client_fd > maxfd)
                    maxfd = client_fd;
                sprintf(buffer, "server: client %d just arrived\n", id);
                broadcat(&writefds, client_fd, maxfd, buffer, strlen(buffer));
                id++;
            }
            else
            {
	            bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
	            if (bytes <= 0)
	            {
                    sprintf(buffer, "server: client %d just left\n", client_ids[fd]);
                    broadcat(&writefds, fd, maxfd, buffer, strlen(buffer));
		            close(fd);
	            	FD_CLR(fd, &clientfds);
	            }
	            else
	            {
                    for (int i = 0; i < bytes; i++)
                    {
                        client_buff[fd][client_len[fd]++] = buffer[i];
                            if (buffer[i] == '\n')
                            {
                                char message[2048];
                                client_buff[fd][client_len[fd]] = '\0';
                                sprintf(message, "client %d: %s", client_ids[fd], client_buff[fd]);

                                broadcat(&writefds, fd, maxfd, client_buff[fd], client_len[fd]);
                                client_len[fd] = 0;
                            }
                    }
	            }
            }
        }
    }
    return 0;
}