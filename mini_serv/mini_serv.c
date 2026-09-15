#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/select.h>

int main(int ac, char **av)
{
    fd_set				clientfds;
    fd_set				currentfds;
    int maxfd;
    int bytes;
    char buffer[1024];

    if (ac != 2)
    {
        write(2, "Wrong number of arguments\n", 26);
        exit(1);
    }

    int listen_socket = socket(AF_INET, SOCK_STREAM, 0);

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

	    if (select(maxfd + 1, &currentfds, 0, 0, 0) < 0)
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

                FD_SET(client_fd, &clientfds);

                if (client_fd > maxfd)
                    maxfd = client_fd;
            }
            else
            {
	            bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
	            if (bytes <= 0)
	            {
		            close(fd);
	            	FD_CLR(fd, &clientfds);
	            }
	            else
	            {
		            buffer[bytes] = '\0';
		            write(1, buffer, bytes);
	            }
            }
        }
    }
    return 0;
}