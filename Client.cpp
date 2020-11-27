#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <iostream>
#include <fstream>
#include <netdb.h>
#include <ctype.h>
using namespace std;

#define PORT 7811
void fileSEND(long connfd, char *fname)
{

    write(connfd, fname, 256);

    FILE *fp = fopen(fname, "rb");
    if (fp == NULL)
    {
        printf("File opern error");
        return;
    }

    /* Read data from file and send it */
    while (1)
    {
        /* First read file in chunks of 256 bytes */
        unsigned char buff[1024] = { 0 };
        int nread = fread(buff, 1, 1024, fp);
        //printf("Bytes read %d \n", nread);        

        /* If read was success, send data. */
        if (nread > 0)
        {
            //printf("Sending \n");
            write(connfd, buff, nread);
        }
        if (nread < 1024)
        {
            fclose(fp);
            if (feof(fp))
            {
                
                printf("End of file\n");
                printf("File transfer completed for id: %ld\n", connfd);
            }
            if (ferror(fp))
                printf("Error reading\n");
            break;
        }
    }
    printf("Closing Connection for id: %ld\n", connfd);
    //while(1);
    //close(connfd);
    //shutdown(connfd, SHUT_WR);
    
    sleep(2);
}

int filereceive(int sockfd)
{
    FILE *fp;
    char recvBuff[1024];
    memset(recvBuff, '0', sizeof(recvBuff));
    char fname[100];
    read(sockfd, fname, 256);
    //strcat(fname,"AK");
    printf("File Name: %s\n",fname);
    printf("Receiving file...");
     fp = fopen(fname, "ab"); 
        if(NULL == fp)
        {
         printf("Error opening file");
         return 1;
        }
    long double sz=1;
    /* Receive data in chunks of 256 bytes */
    while(1)
    { 
    int nread = read(sockfd, recvBuff, 1024);
    fflush(stdout);
    if (nread > 0)
    {
        fwrite(recvBuff, 1,nread,fp);
    }
        if (nread < 1024)
    {
        fclose(fp);
        if (feof(fp))
        {
            
            printf("End of file\n");
            printf("File transfer completed for id: %d\n", sockfd);
        }
        if (ferror(fp))
            printf("Error reading\n");
        break;
    }
    }
    fclose(fp);
    
    printf("\nFile OK....Completed\n");
    return 0;
}

void RetrieveFile(int fd)
{   char buffer[1024];
    cout<<"Here in retrieve function"<<endl;
    if(recv(fd,buffer,1024,0) < 0)              // first receieve the message
    {
        printf("[-]Error in receiving data.\n");
    }        
    cout<<buffer<<endl;

    recv(fd,buffer,1024,0);         // receieve list of files
    cout<<buffer<<endl;

    printf("Client: \t");
    bzero(buffer, sizeof(buffer));
    char hmmm[100];
    scanf("%s", hmmm);        
    send(fd, hmmm, strlen(hmmm), 0); // client sends file name
    cout<<"Going to recieve file"<<endl;
    filereceive(fd);

}
int main(int argc,char *argv[])
{

    if(argc<2)
    {
        cout<<"IP or PORT no. is not provided exiting....."<<endl;
        exit(1);
    }
    int port = atoi(argv[2]);
    struct  hostent * server;
    server = gethostbyname(argv[1]);   


	int clientSocket, ret;
	struct sockaddr_in serverAddr;
	char buffer[1024];

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Client Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	//serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    bcopy((char *)server->h_addr,(char *)&serverAddr.sin_addr.s_addr,server->h_length);


	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Connected to Server.\n");
//
	while(1)
	{
		printf("Client: \t");
		bzero(buffer, sizeof(buffer));
		scanf("%s", buffer);
		if(strncmp(buffer,"440",3)==0)
		{
		 	int x=440;
            cout<<"Enter filename you want to send: ";
            string filename;
            cin>>filename;
            char fName[filename.size()+1];
            strcpy(fName,filename.c_str());
		 	send(clientSocket, &x, sizeof(int), 0);
		 	fileSEND(clientSocket,fName);
            cout<<"\nFile sent";
            close(clientSocket);
            exit(1);

		}
		if(strncmp(buffer,"450",3)==0)
		{
            int x=450;
            send(clientSocket, &x, sizeof(int), 0);
            cout<<"450 recievd"<<endl;
		 	RetrieveFile(clientSocket);
            close(clientSocket);
            exit(1);
            
		}
		send(clientSocket, buffer, strlen(buffer), 0);
		bzero(buffer, sizeof(buffer));
		if(recv(clientSocket, buffer, 1024, 0) < 0)
		{
			printf("[-]Error in receiving data.\n");
		}
		else
		{
			printf("Server: \t%s\n", buffer);
			bzero(buffer, sizeof(buffer));
			memset(buffer,'\0',1024);
		}
	}

	close(clientSocket);
	return 0;
}
