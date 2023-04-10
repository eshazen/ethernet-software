
This directory contains a sample client/server pair.

  client.c
  server.c

This is the simplest version.  The server binds to port 5432
(arbitrary constant in the code) and listens for connections.
The client connects to this port on the specified host,
and then reads strings from standard input, which are output
by the server.

  bi_client.c
  bi_server.c

This is a slightly more complex example which sends back a return
message from the server for each transmitted string

No special compile switches are needed... just type 'make client'
or 'make server' on most unix systems, or
  cc -o client client.c
  cc -o server server.c


