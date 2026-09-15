#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>

int main(int ac, char **av)
{
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
	// close(listen_socket);

    return 0;
}