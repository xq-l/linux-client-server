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



using namespace std;


void process_connn_child(int s,int request_bytes){

	ssize_t size = 0;
	
	char buffer[1024];
	char request[1024];

	snprintf(request,sizeof(request),"%d\n",request_bytes);
	cout<<endl<<"request server "<<request_bytes<<" bytes"<<endl;
    write(s,request, strlen(request)+1);

    int nleft = request_bytes;
	
	while( nleft > 0){

		size = read(s,buffer,request_bytes);

		if(size > 0){
			nleft -= size;
			/*
			string s1 = string("server response: ");
			string s2 = string(buffer);
			string s = ""+s1+s2;
			//write(1,s.c_str(),s.size());
			cout<<endl<<s<<endl;*/
			cout<<endl<<"server response "<< size<<" bytes"<<endl;
		}
		else{
			cout<<"server response 0 bytes"<<endl;
			break;
		}
		
	}

}

int tcp_connect(string sip,int nport){

	struct sockaddr_in server_addr;
	int s;
	s = socket(AF_INET,SOCK_STREAM,0);
	if(s < 0){
		perror("socket");
		exit(-1);
	}

	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(nport);

	inet_pton(AF_INET, sip.c_str(),&server_addr.sin_addr);

	connect(s,(struct sockaddr *)&server_addr,sizeof(struct sockaddr));

	return s;

}

int main(int argc, char **argv){


	

	if(argc != 6){
		cout<<"usage: client <hostname or IPaddr> <port> <children> <loops/child> <bytes/request>"<<endl;
		exit(-1);
	}

    string sip = string(argv[1]);
    int nport = atoi(argv[2]);
    int nchildren = atoi(argv[3]);
	int nloops = atoi(argv[4]);
	int nbytes = atoi(argv[5]);

   	int s;	
	
	for(int i = 0; i < nchildren; ++i){
		pid_t pid;
		if( (pid = fork()) == 0 ){
			
			for(int j = 0; j < nloops; ++j){

				s = tcp_connect(sip,nport);

				process_connn_child(s,nbytes);

				close(s);

			}
						
			exit(0);
		}



	}

	while( wait(NULL) > 0);
	
	exit(0);
}

