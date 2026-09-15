# EXAM_06
# mini_serv

A small non-blocking TCP server written in C for the Rank 06 exam.

## Assignment

The server listens on `127.0.0.1` on the port given as its first argument.

Clients can connect and send messages. The server broadcasts messages to the other connected clients.

## Constraints

- Language: C
- Expected file: `mini_serv.c`
- No `#define` preprocessor directives
- Only the functions listed in the subject are allowed
- The server must listen only on `127.0.0.1`
- The server must use `select()`
- The server must not disconnect a client just because it is slow to read
- Memory and file descriptors must be cleaned up

## Main concepts

- TCP sockets
- File descriptors
- `bind()` and `listen()`
- `accept()`
- `select()`
- Non-blocking event-driven programming
- Client IDs
- Broadcasting
- Partial `send()`
- Memory management

## Checkpoint 01 

The program now requires exactly one argument after the executable name.

Behavior
./mini_serv
    → Wrong number of arguments
    → exit status 1

./mini_serv 8081
    → accepted
    → exit status 0

The subject requires the exact error message:

Wrong number of arguments followed by a newline.

We use write().

## Checkpoint 02 - Create the TCP socket

The program now creates an IPv4 TCP socket using:

socket(AF_INET, SOCK_STREAM, 0);
What socket() does

socket() asks the operating system to create a communication endpoint and returns a file descriptor.

Example:

socket()
   ↓
Operating System
   ↓
fd = 3

The FD number is not fixed; it is assigned by the operating system.

Arguments
AF_INET

Selects IPv4.

SOCK_STREAM

Selects a reliable byte-stream socket, which corresponds to TCP.

0

Lets the operating system choose the appropriate protocol for the requested socket type.

Error handling

If socket() fails, the assignment requires:

Fatal error followed by a newline, written to stderr, and the program exits with status 1.


## Checkpoint 03  — Bind the socket

The socket is now associated with the required local address.

The server must listen only on:

```text
127.0.0.1
```

### `struct sockaddr_in`

For IPv4, the socket address is stored in:

```c
struct sockaddr_in
```

Important fields:

```text
sin_family
    → address family

sin_port
    → server port

sin_addr.s_addr
    → server IP address
```

### `bzero()`

The structure is cleared before its fields are initialized:
This prevents uninitialized data from remaining in the structure.

### `bind()`

`bind()` associates the socket with a local IP address and port.

The general flow is:

```text
socket()
    ↓
create TCP socket
    ↓
bind()
    ↓
assign 127.0.0.1 + PORT
```

The socket is not listening yet.

## Checkpoint 04 — Listen for connections

The socket is now configured as a listening TCP socket using:

listen();

bind() associates the socket with 127.0.0.1 and the requested port.

listen() then tells the operating system that this socket is ready to receive incoming TCP connection requests.

The flow is now:

socket()
    ↓
create TCP socket
    ↓
bind()
    ↓
assign 127.0.0.1 + PORT
    ↓
listen()
    ↓
wait for connection requests

The listening socket is not the socket used to communicate with a client.

When a client connects, accept() will later create a separate socket for that client.

listen()

The call is:

listen(sockfd, backlog);

The first argument is the listening socket.

The second argument is the backlog, which specifies the queue for pending connection requests that have not yet been accepted by the server.

Error handling

If listen() fails:

Fatal error is written to stderr, the listening socket is closed, and the program exits with status 1.

At this point, the server can listen for incoming connections, but it does not accept or communicate with clients yet.
