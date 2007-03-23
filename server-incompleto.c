/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define MYPORT 8642	// the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold



int main(void){

  fd_set master, read_fds;
  int fdmax;
  int nbytes;
  char buf[256];
  int listener, new_fd;  // listen on sock_fd, new connection on new_fd
  struct sockaddr_in my_addr;	// my address information
  struct sockaddr_in remote_addr; // connector's address information
  socklen_t sin_size;
  struct sigaction sa;
  int yes=1;

  FD_ZERO(&master);
  FD_ZERO(&read_fds);

  if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

  if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
  }
	
  my_addr.sin_family = AF_INET; // host byte order
  my_addr.sin_port = htons(MYPORT);	 // short, network byte order
  my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
  memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct
  
  if (bind(listener, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
    perror("bind");
    exit(1);
  }
  
  
  if (listen(listener, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }
  
 
  FD_SET(listener,&master);  
  fdmax = listener;
  
  while(1) {
    read_fds = master;
  
    if(select(fdmax+1,&read_fds,NULL,NULL,NULL)== -1) {
      perror("select");
      exit(1);
    }
    
    for(int i=0;i<=fdmax;i++) {
      
      if(FD_ISSET(i,&read_fds)) {

	if(i==listener) {
	  
	  sin_size = sizeof(struct sockaddr_in);
	  if ((new_fd = accept(listener, (struct sockaddr *)&remote_addr, &sin_size)) == -1) {
	    perror("accept");			
	  }else {
	    
	    //add your code

	    printf("server: got connection from %s fd:%d\n",inet_ntoa(remote_addr.sin_addr),i);
	  }
	
	  
	}else {
	  
	  if((nbytes = recv(i,buf,sizeof(buf),0))<=0) {
	    
	    if(nbytes==0) {	    
	      printf("server: client %d hung up\n",i);
	    }else { 
	      //add your code	    
	    };
	    
	    //add your code

	  }else {	    	   
	    //add your code
	  }		
	}
      }
    }
  }
  return 0;

}

