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