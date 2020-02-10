//
// Created by Hanyi Wang on 2/4/20.
//

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>

int main(int argc, char **argv) {

    // Extract arguments
    // First check the input arguments
    if (argc != 5) {
        perror("Input arguments should be 5!!! \n");
        abort();
    }

    // 1. Get host name
    struct hostent * host = gethostbyname(argv[1]);
    if (!host) {
        perror("invalid server host name!!! \n");
        abort();
    }
    // Get server's ip address
    unsigned int server_addr = *(unsigned int *) host->h_addr_list[0];

    // 2. Get port address
    unsigned short port_number = atoi(argv[2]);
    if (port_number < 18000 || port_number > 18200) {
        perror("Input port address is not valid!!! \n");
        abort();
    }

    // 3. Get message size
    unsigned short size = atoi(argv[3]);
    if (size < 10 || size > 65535) {
        perror("Input message size is not valid!!! \n");
        abort();
    }

    // 4. Get count
    int count = atoi(argv[4]);
    if (count < 1 || count > 10000) {
        perror("Input count is not valid!!! \n");
        abort();
    }

    // allocate memory for send and recv buffer
    char * recv_buffer = (char *) malloc(size);
    char * send_buffer = (char *) malloc(size);
    if (!recv_buffer || !send_buffer) {
        perror("failed to allocated buffers");
        abort();
    }

    // Create socket
    int sock;
    struct sockaddr_in sin;
    memset (&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = server_addr;
    sin.sin_port = htons(port_number);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Unable to create socket!!! \n");
        abort();
    }

    // Try Socket Connection
    if (connect(sock, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
        perror("connect to server failed");
        abort();
    }

    // Connection is Ready
    // Initialize the buffer
    *(short*) send_buffer = (short)htons(size);

    long double total_latency = 0.0;
    long double average_latency = 0.0;
    struct timeval start,end;

    int i = 0;
    while (i++ < count) {
        // Start timing
        gettimeofday(&start, NULL);
        *(int*) (send_buffer + 2) = (int)htons(start.tv_sec);
        *(int*) (send_buffer + 2 + 4) = (int)htons(start.tv_usec);

        int send_size = 0;
        int recv_size = 0;
        while(send_size < size){
            send_size += send(sock,send_buffer + send_size,size - send_size, 0);
        }

        while(recv_size < size){
            recv_size = recv(sock, recv_buffer + recv_size,size - recv_size, 0);
        }

        // End timing
        gettimeofday(&end, NULL);
        total_latency += 1000000 * (end.tv_sec - start.tv_sec) +(end.tv_usec-start.tv_usec);
    }

    average_latency = total_latency / count / 1000;
    printf("Total latency for count %d of %Lf \n", count ,total_latency);
    printf("Average latency for count %d of %Lf \n", count ,average_latency);

    close(sock);
    free(recv_buffer);
    free(send_buffer);

    return 0;
}
