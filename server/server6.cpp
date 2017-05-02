#include <iostream>
#include <string>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <sys/resource.h>
#include <pthread.h>
#include <time.h>
#include <sys/select.h>


using namespace std;

#define BACKLOG 10 

#define CLIENTNUM 1024


static long long  connect_times = 0;
 
static int connect_host[CLIENTNUM];
static int connect_number = 0;

pthread_mutex_t ALOCK = PTHREAD_MUTEX_INITIALIZER;

#ifndef	HAVE_GETRUSAGE_PROTO
int		getrusage(int, struct rusage *);
#endif
void pr_cpu_time(void){

	double			user, sys;
	struct rusage	myusage, childusage;

	if (getrusage(RUSAGE_SELF, &myusage) < 0)
		cout<<"getrusage error"<<endl;
	if (getrusage(RUSAGE_CHILDREN, &childusage) < 0)
		cout<<"getrusage error"<<endl;

	user = (double) myusage.ru_utime.tv_sec +
					myusage.ru_utime.tv_usec/1000000.0;
	user += (double) childusage.ru_utime.tv_sec +
					 childusage.ru_utime.tv_usec/1000000.0;
	sys = (double) myusage.ru_stime.tv_sec +
				   myusage.ru_stime.tv_usec/1000000.0;
	sys += (double) childusage.ru_stime.tv_sec +
					childusage.ru_stime.tv_usec/1000000.0;
	printf("\nuser time = %g, sys time = %g\n", user, sys);
}

void sig_int(int signo){

	
	pr_cpu_time();


	exit(0);
}


void process_conn_server( int sc){

	
	ssize_t size = 0;

	char buffer[1024];
	
	

	read(sc,buffer,1024);

	long  request_bytes = atol(buffer);
	cout<<"clinet request "<<request_bytes<<" bytes"<<endl;

	string respone = string(request_bytes,'a');

	long nleft = request_bytes;

	
	while(nleft > 0){
		
		size = write(sc,respone.c_str(), request_bytes);
		if(size > 0){
			nleft -= size;
			cout<<"server response "<<size<<" bytes"<<endl;
		}
		else{
			cout<<"server response 0 bytes"<<endl;
			break;
		}
	}
	
	
}


void *handle_connect(void *argv){

	pthread_detach(pthread_self());
	
	int ss = *(int *)argv;

	int connfd;
	socklen_t chilen;
	struct sockaddr_in client_addr;

	fd_set listenfd;
	struct timeval timeout;
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;
	int maxfd = -1;
	int err = -1;
	
	while(1){

		maxfd = -1;
		FD_ZERO(&listenfd);
		FD_SET(ss,&listenfd);
		if(maxfd < ss){
			maxfd = ss;
		}
		err = select(maxfd+1,&listenfd,NULL,NULL,&timeout);

		switch(err){
			case 0:break;
			case -1:perror("select");break;
			default:{
				if( FD_ISSET(ss,&listenfd) ){
					connfd = accept(ss,(struct sockaddr *)&client_addr,&chilen);
		
					if(connfd < 0){
 						if(errno == EINTR)
    						continue;
    					else
    					perror("accept");
    				}
    				cout<<"connect times : "<< ++connect_times << endl;
					char cip[16];
    				cout<<"clinet ip:"<<inet_ntop(AF_INET,&client_addr.sin_addr, cip,16)<<endl;
    				cout<<"clinet port:"<<client_addr.sin_port<<endl;
    		
    				pthread_mutex_lock(&ALOCK);
    				for(int i = 0; i < CLIENTNUM; ++i){
    					if(connect_host[i] == -1){
    						connect_host[i] = connfd;
    						++connect_number;
    						break;
    					}

    				}
    				pthread_mutex_unlock(&ALOCK);
				}

				break;
			}


		}	
	}
	return NULL;
}

void *handle_request(void * argv){

	

	int maxfd = -1;
	int err = -1;
	fd_set scanfd;
	struct timeval timeout;
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;
	while(1){

		maxfd = -1;
		FD_ZERO(&scanfd);
		pthread_mutex_lock(&ALOCK);
		for(int i = 0;  i < CLIENTNUM; ++i){
			if(connect_host[i] != -1){
				FD_SET(connect_host[i],&scanfd);
				if(maxfd < connect_host[i]){
					maxfd = connect_host[i];
				}

			}
		}
		pthread_mutex_unlock(&ALOCK);

		err = select(maxfd+1,&scanfd,NULL,NULL,&timeout);
		switch(err){
			case 0:break;
			case -1:  perror("select"); break;
			default:{
						if( connect_number <= 0)
							break;
						pthread_mutex_lock(&ALOCK);
						for(int i = 0; i < CLIENTNUM; ++i){
							if( connect_host[i] != -1){
								if( FD_ISSET(connect_host[i],&scanfd) ){
									
									process_conn_server(connect_host[i]);

									close(connect_host[i]);
									
									connect_host[i] = -1;
									--connect_number;

								}
							}

						}
					   pthread_mutex_unlock(&ALOCK);

					break;
			}

		}


	}
	return NULL;
}

int main(int argc,char ** argv){

	if(argc != 3){
		cout<<"usage: server <hostname or IPaddr> <port>"<<endl;
		exit(-1);
	}

	string sip = string(argv[1]);
	int nport = atoi(argv[2]);

	int ss;
	struct sockaddr_in server_addr;
	int err;

	ss = socket(AF_INET,SOCK_STREAM,0);
	if(ss < 0){
		perror("socker");
		exit(-1);
	}
	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(nport);
	inet_pton(AF_INET, sip.c_str(),&server_addr.sin_addr);

    err = bind(ss,(struct sockaddr *)&server_addr,sizeof(server_addr));
    if(err < 0){
    	perror("bind");
    	exit(-1);
    }

    err = listen(ss,BACKLOG);
    if(err < 0){
    	perror("linten");
    	exit(-1);
    }


    signal(SIGINT, sig_int);

    memset(connect_host,-1,CLIENTNUM);

    pthread_t thread_do[2];

    pthread_create(&thread_do[0],NULL,handle_connect,(void *)&ss);
    pthread_create(&thread_do[1],NULL,handle_request, NULL);

    
	    
  
	 while(1){
	 	;
	 }

	close(ss);

    exit(0);
}
