The code was created and test on macOS 10.15.3. Please excuse any compatibility issues if run on Ubuntu.

Execution Instructions in order are as follows:-
    1. Put an input file "input.txt" in the folder
    2. gcc -o server server.c
    3. gcc -o client client.c
    4. ./server
    5. ./client
    (On 2 separate terminals)

P.S. 
-The client program will quit automatically on completion while server has to be quit manually after other programs have quit, use Ctrl+C.

Methodology

I created 2 separate channels by creating 2 sockets in the client file.
Acknowlegements on the client side are handled using polling with a fixed timeout.
If there's no event till the timeout the packet is transmitted again.
In order to ensure that there are atmost 2 unacknowledged packets, if a packet loss is detected by timeout, we don't transmit the next packet from that channel until it has been successfully sent. 
The sequence numbers of packets on the server side are checked. In case any sequence number is bigger than the expected one, it is buffered.
The contents of this buffered packet are added to the file once the missing packet is received.