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

## Checkpoint 05 — Wait for events with select()

The server now uses select() to wait for activity on the listening socket.

fd_set

An fd_set is used to keep track of the file descriptors that the server wants to monitor.

fd_set fds;

The set is initialized with:

FD_ZERO();

Then the listening socket is added:

FD_SET(sockfd, &fds);

The server can now wait for activity.

What select() does

select() blocks until one or more monitored file descriptors becomes ready.

At this point, only the listening socket is monitored.

When a client attempts to connect, the listening socket becomes ready for reading, which means that accept() can be called.

The flow is:

socket()
    ↓
bind()
    ↓
listen()
    ↓
FD_ZERO()
    ↓
FD_SET(sockfd)
    ↓
select()
    ↓
waiting for a connection

Important behavior

select() does not accept the connection itself.

It only tells the server that the listening socket is ready.

Error handling

If select() returns a negative value, the listening socket is closed, Fatal error is written to stderr, and the program exits with status 1.

## ## Checkpoint 06 — Accept connections

The server can now accept an incoming TCP connection using `accept()`.

After `select()` reports that the listening socket is ready, the server calls:

```c
clientfd = accept(sockfd, 0, 0);
```

### Listening socket vs client socket

The listening socket and the client socket have different responsibilities.

```text
                    SERVER
                      │
                listening socket
                    sockfd
                      │
                   listen()
                      │
                  select()
                      │
                connection arrives
                      │
                   accept()
                      │
                      ▼
                 client socket
                   clientfd
```

The listening socket stays open so the server can accept more connections.

`accept()` creates a **new file descriptor** dedicated to communicating with that particular client.

For example:

```text
sockfd  = 3    → listening socket
clientfd = 4   → Client 0
```

If another client connects:

```text
sockfd  = 3    → listening socket
clientfd = 5   → Client 1
```

The listening socket remains `3`.

### Adding the client to `select()`

After accepting a client, its file descriptor is added to the master `fd_set`:

This tells the server:

> "I also want to monitor this client for incoming data."

The server also updates `maxfd` if necessary:

```c
if (clientfd > maxfd)
	maxfd = clientfd;
```

This is needed because `select()` expects the highest monitored file descriptor plus one.

### Current flow

```text
socket()
    ↓
bind()
    ↓
listen()
    ↓
select()
    ↓
connection arrives
    ↓
accept()
    ↓
new clientfd
    ↓
FD_SET(clientfd, &readfds)
    ↓
select() again
```

At this point, the server can accept a client connection and keep its socket in the set of monitored file descriptors.

The next step is to detect when an **existing client socket** is ready and use `recv()` to read its data.
