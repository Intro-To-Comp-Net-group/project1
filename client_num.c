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
/* simple client, takes four parameters, the server domain name,
   and the server port number 8000 <= port <= 18200,the maxbytesize for each information trans(10-65535),the count of info to be transed(1,10000)*/
void allocatebuffer(char** buffer,char** sendbuffer,int bytesize){
  *buffer = (char *) malloc(bytesize);
  if (!(*buffer))
    {
      perror("failed to allocated buffer");
      abort();
    }

  *sendbuffer = (char *) malloc(bytesize);
  if (!(*sendbuffer))
    {
      perror("failed to allocated sendbuffer");
      abort();
    }
}

void setTimeStamp(char* sendbuffer, struct timeval* tv){
	gettimeofday(tv,NULL);
	*(int*)(sendbuffer + 2) = (int)htons(tv->tv_sec);
	*(int*)(sendbuffer + 2 + 4) = (int)htons(tv->tv_usec);
}

int main(int argc, char** argv) {

  /* our client socket */
  int sock;
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

  if(argc != 5 ||server_port<0 || server_port >10000 || bytesize <10 || bytesize > 65535 || countamount <1 || countamount > 10000){
    perror("Invalid input parameters");
    abort();
  }

  long double total_latency,average_latency;
  char* buffer = NULL;
  char* sendbuffer = NULL;

  /* allocate a memory buffer in the heap */
  /* putting a buffer on the stack like:

         char buffer[500];

     leaves the potential for
     buffer overflow vulnerability */
  allocatebuffer(&buffer,&sendbuffer,bytesize);

  printf("allocate buffer");

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


  total_latency = 0.0;
  average_latency = 0.0;
  struct timeval start,end;

  /*fill the first buffer of bytesize*/
  *(short*)sendbuffer = (short)htons(bytesize);

  for(int i = 0;i<countamount;i++){
    //struct timeval* startptr = ;
    setTimeStamp(sendbuffer, &start);

    int t = 0;
    while(t < bytesize){
      t += send(sock,sendbuffer + t,bytesize - t,0);
      // printf("%d\n",t);
    }

    t = 0;

    while(t < bytesize){
      /*??*/
      t += recv(sock,buffer+t,bytesize-t,0);
      // printf("receive data %d\n",t);
    }
    gettimeofday(&end,NULL);

    long double current = (end.tv_sec - start.tv_sec) *pow(10.0,6)+(end.tv_usec-start.tv_usec);
    total_latency += current;
  }


  average_latency = total_latency/(countamount * pow(10.0,3));

  printf("Average latency for countamount %d of %Lf seconds\n",countamount,average_latency);

  close(sock);
  free(buffer);
  free(sendbuffer);
  return 0;

}

