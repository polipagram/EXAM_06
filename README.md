# EXAM_06 — mini_serv

A small non-blocking TCP chat server written in C for the 42 Rank 06 exam.

## What is required?

The server must:

* Listen only on `127.0.0.1` using the port passed as an argument.
* Use `socket()`, `bind()`, `listen()`, `accept()`, `recv()`, `send()` and `select()`.
* Handle multiple clients in one event-driven loop.
* Assign each client an incremental ID.
* Broadcast messages to every other connected client.
* Correctly handle partial messages and multiple lines received together.
* Avoid blocking or disconnecting slow-reading clients.
* Free allocated memory and close client sockets.
* Print the exact required error messages.

## Important tips

* Keep a master `fd_set` and copy it before calling `select()`.
* Use a separate dynamic buffer for each client.
* TCP is a byte stream: one `recv()` does not necessarily equal one message.
* Process complete messages only when `\n` is received.
* Use the writable set from `select()` when broadcasting.
* The client ID and file descriptor are different things.

## Validation

This version was compiled and manually validated with multiple clients, message broadcasting, multiple lines, partial messages, and client disconnections.
