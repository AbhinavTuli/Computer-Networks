The code was created and test on macOS 10.15.3. Please excuse any compatibility issues if run on Ubuntu.

Execution Instructions in order are as follows:-

    1. Put an input file "input.txt" in the folder
    2. gcc -o server server.c
    3. gcc -o relay relay.c
    4. gcc -o client client.c
    5. ./server
    6. ./relay
    7. ./client
    (On 3 separate terminals)

P.S. 
-The client and server program will quit automatically on completion while relay has to be quit manually after other programs have quit, use Ctrl+C.
-The implementation of TIME has not been done in the specified format as I was facing certain issues. Instead, the time printed is in microseconds since 1/1/1970.


Methodology
Both of the relays are present in a single file. All acknowledgments are sent back through relay 1. 
At the client side, a sliding window is maintained using a linkedlist, to allow for easier insertion and deletion. This sliding window keeps packets and maintains a record of whether they have been acknowledged.
The head of this linkedlist keeps shifting.
The window size has been taken as 4 and sequence numbers from 0 to 7 have been used.

The client as well as the relays keep polling, with a timeout of 0 for acknowledgements.
When client receives acknowledgement, the sliding window is scanned to find the relevant entry and that entry is marked as having gotten acknowledgement.
The window shifts whenever it finds 1 or more consecutive acknowledged packets at the start.

As the relays can cause a delay of upto 2 ms, the timout on the client side has been kept as 3 ms. Once this expires, the first packet in the window is retransmitted.

On the server side, two buffers of size 8*90 i.e. total buffer size is 1440 characters are maintained to write to the file in order. Once the first buffer is full or the end has been reached, the buffer writes its content to the file and the value of second buffer is copied to the first one.
I have tested this with files of size of over 19000 characters, which is more than 10x the buffer size.


