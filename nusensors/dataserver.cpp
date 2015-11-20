#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define maxlen 1024
#define port 6789

int wgs_atoi(char * str)
{
    int ret=0;
    while((*str)!='\0')
    {
        ret=((*str)-'0') + ret*10;
        str++;
    }
    return ret;
}

int main(int argc, char** argv) {

	int flag=0;
	FILE * fp;
	pid_t pid1;
	pid_t pid2;
    //int sin_len;
    socklen_t sin_len;
    char message[256];

    int socket_descriptor;
    struct sockaddr_in sin;
    printf("Waiting for data form sender \n");

    bzero(&sin,sizeof(sin));
    sin.sin_family=AF_INET;
    sin.sin_addr.s_addr=htonl(INADDR_ANY);
    sin.sin_port=htons(port);
    sin_len=sizeof(sin);

    socket_descriptor=socket(AF_INET,SOCK_DGRAM,0);
    bind(socket_descriptor,(struct sockaddr *)&sin,sizeof(sin));
	
	printf("the main argc=%d\n",argc);
	//./0dataserver 1-g 2num 3-s 4waitnum 5num
	printf("-g argv[2]=%s\n",argv[2]);
	printf("-s argv[4]=%s\n",argv[4]);
	printf("-s argv[5]=%s\n",argv[5]);
	flag=0;
	fp=fopen("/data/allserverdata.txt","w+");

	pid1=fork();
	if(pid1<0)
	{
		printf("fork error!\n");
		exit(1);
	}
	else if(pid1 == 0)
	{
		printf("child process\n");
		if(execl("mytestgps","mytestgps","-g",argv[2],"stop",NULL)<0)
		{
			fprintf(stderr,"execl failed: %s\n",strerror(errno));
			exit(0);
		}
	}

	pid2=fork();
	if(pid2<0)
	{
		printf("fork error!\n");
		exit(1);
	}
	else if(pid2 == 0)
	{
		printf("child process\n");
		if(execl("mytestsensor","mytestsensor","-g",argv[4],argv[5],NULL)<0)
		{
			fprintf(stderr,"execl failed: %s\n",strerror(errno));
			exit(0);
		}
	}
	printf("server start to waite\n");
    while(1)
    {
        recvfrom(socket_descriptor,message,sizeof(message),0,(struct sockaddr *)&sin,&sin_len);
        //printf("Response from server:%s\n",message);
	//fprintf(fp,"Response from server:%s\n",message);
	fprintf(fp,"%s",message);
        if(strncmp(message,"stop",4) == 0)//接受到的消息为 “stop”
        {
		flag++;
            //printf("Sender has told me to end the connection\n");
            if(flag==2) break;
        }
    }

    close(socket_descriptor);
	fclose(fp);
    exit(0);

    return (EXIT_SUCCESS);
}
