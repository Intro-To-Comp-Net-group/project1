#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <math.h>
/* simple client, takes two parameters, the server domain name,
   and the server port number */

int main(int argc, char** argv) {

  /* our client socket */
  int sock;
  printf("getyou");
  /* variables for identifying the server */
  unsigned int server_addr;
  struct sockaddr_in sin;
  struct addrinfo *getaddrinfo_result, hints;

  /* convert server domain name to IP address */
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET; /* indicates we want IPv4 */

  if (getaddrinfo(argv[1], NULL, &hints, &getaddrinfo_result) == 0) {
    server_addr = (unsigned int) ((struct sockaddr_in *) (getaddrinfo_result->ai_addr))->sin_addr.s_addr;
    freeaddrinfo(getaddrinfo_result);
  }

  /* server port number */
  unsigned short server_port = atoi (argv[2]);
  unsigned short bytesize = atoi(argv[3]);
  int countamount = atoi(argv[4]);

  long double total_latency,average_latency;
  char *buffer, *sendbuffer;

  /* allocate a memory buffer in the heap */
  /* putting a buffer on the stack like:

         char buffer[500];

     leaves the potential for
     buffer overflow vulnerability */
  buffer = (char *) malloc(bytesize);
  if (!buffer)
    {
      perror("failed to allocated buffer");
      abort();
    }

  sendbuffer = (char *) malloc(bytesize);
  if (!sendbuffer)
    {
      perror("failed to allocated sendbuffer");
      abort();
    }


  /* create a socket */
  if ((sock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
      perror ("opening TCP socket");
      abort ();
    }

  /* fill in the server's address */
  memset (&sin, 0, sizeof (sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = server_addr;
  sin.sin_port = htons(server_port);

  /* connect to the server */
  if (connect(sock, (struct sockaddr *) &sin, sizeof (sin)) < 0)
    {
      perror("connect to server failed");
      abort();
    }

  /* everything looks good, since we are expecting a
     message from the server in this example, let's try receiving a
     message from the socket. this call will block until some data
     has been received */
  /*
  count = recv(sock, buffer, size, 0);
  if (count < 0)
    {
      perror("receive failure");
      abort();
    }
  if (buffer[count-1] != 0)
    {
      printf("Message incomplete, something is still being transmitted\n");
    } 
  else
    {
      printf("Here is what we got: %s", buffer);
    }

  printf("getyou1");
  */

  total_latency = 0.0;
  average_latency = 0.0;
  struct timeval start,end;

  /*fill the first buffer of bytesize*/
  *(short*)sendbuffer = (short)htons(bytesize);
  
  printf("client bytesize");

  for(int i = 0;i<countamount;i++){
    //struct timeval* startptr = ;
    gettimeofday(&start,NULL);
    *(int*) (sendbuffer + 2) = (int)htons(start.tv_sec);
    *(int*) (sendbuffer + 2 + 4) = (int)htons(start.tv_usec);

    int t = 0;
    while(t < bytesize){
      t += send(sock,sendbuffer + t,bytesize - t,0);
    }

    t = 0;
    while(t < bytesize){
      t = recv(sock,sendbuffer+t,bytesize-t,0);
    }
    gettimeofday(&end,NULL);

    long double current = (end.tv_sec - start.tv_sec) *pow(10.0,6)+(end.tv_usec-start.tv_sec);
    total_latency += current;
  }


  average_latency = total_latency/countamount;

  printf("Average latency for countamount %d of %Lf \n",countamount,average_latency);

  close(sock);
  free(buffer);
  free(sendbuffer);
  return 0;

}
