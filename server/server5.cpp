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


using namespace std;

#define BACKLOG 10 

#define PTHREADCOUNT 5

pthread_mutex_t ALOCK = PTHREAD_MUTEX_INITIALIZER;


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


static void *doit(void * arg){

//	pthread_detach(pthread_self());

	int ss = *(int *)(arg);
	int sc;
	struct sockaddr_in client_addr;
	socklen_t addrlen;
	
	cout<<"child "<<pthread_self()<<" starting!"<<endl;
 	
 	while(1){
 		pthread_mutex_lock(&ALOCK);
 		sc = accept(ss,(struct sockaddr *)&client_addr, &addrlen);
 		if(sc < 0){
 			if(errno == EINTR)
    			continue;
    		else
    			perror("accept");
    	}
 		pthread_mutex_unlock(&ALOCK);
		
		char cip[16];
    	cout<<"clinet ip:"<<inet_ntop(AF_INET,&client_addr.sin_addr, cip,16)<<endl;
    	cout<<"clinet port:"<<client_addr.sin_port<<endl;
		
		process_conn_server(sc);
		close(sc);
 	
 	}
	return NULL;

}

void handle_connect(int s){

	int ss = s;
	pthread_t thread_do[PTHREADCOUNT];
	
	for(int i = 0; i < PTHREADCOUNT;++i){
		pthread_create(&thread_do[i], NULL, doit,(void *)&ss);
	}
	
	for(int i = 0; i < PTHREADCOUNT; ++i){
		pthread_join(thread_do[i],NULL);

	}


}

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

int main(int argc,char **argv){
	
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
    
    handle_connect(ss);
    close(ss);

	return 0;
}

