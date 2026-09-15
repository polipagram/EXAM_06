#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>

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

    return 0;
}