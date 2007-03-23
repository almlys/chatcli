/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 8642 // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

int main(int argc, char *argv[])
{
  int fdmax;
   fd_set master, read_fds;
   int sockfd, numbytes;  
   char buf[MAXDATASIZE];
   struct hostent *he;
   struct sockaddr_in remote_addr; // connector's address information 

   if (argc != 2) {
     fprintf(stderr,"usage: client hostname\n");
     exit(1);
   }
   
   if ((he=gethostbyname(argv[1])) == NULL) {  // get the host info 
     perror("gethostbyname");
     exit(1);
   }
   
   if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
     perror("socket");
     exit(1);
   }

   remote_addr.sin_family = AF_INET;    // host byte order 
   remote_addr.sin_port = htons(PORT);  // short, network byte order 
   remote_addr.sin_addr = *((struct in_addr *)he->h_addr);
   memset(&(remote_addr.sin_zero), '\0', 8);  // zero the rest of the struct 

   if (connect(sockfd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1) {
     perror("connect");
     exit(1);
   }
   
   //reset 
   FD_ZERO(&master);
   FD_ZERO(&read_fds);

   //set
   FD_SET(0,&master);
   FD_SET(sockfd,&master);

   fdmax = sockfd;

   //connection successful
   while(1) {
     read_fds = master; 
     if(select(fdmax+1,&read_fds,NULL,NULL,NULL)== -1) {
       perror("select");
       exit(1);
     }
     
     if(FD_ISSET(0,&read_fds)) {
      
       numbytes=read(0,buf,MAXDATASIZE-1);
       
       if(send(sockfd,buf,numbytes,0) == -1 ){
	 //add your code...
       }      
       
     }else {
      
       if(!FD_ISSET(sockfd,&read_fds)) {
	 //add your code...
       }
       
       
       if((numbytes = recv(sockfd,buf,sizeof(buf),0))<=0) {
	 //add your code...
       }

       //add your code..	
     }
   }
      
   close(sockfd);
   
   return 0;
}

