#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <netdb.h>
#include "pthread.h"
#include <ctype.h>
#define PORT 7811
using namespace std;
struct auth
{
	int tocken = -1;
	string cliName =" ";
};
auth authenticate(int i);
auth signup(int h);

void *ClientConnect(void *arg);
void *AndroidDeviceAsClient(void *arg);
int parseARGS(char **args, char *line)
{
    int tmp = 0;
    args[tmp] = strtok(line, ":" );
    while ((args[++tmp] = strtok(NULL, ":" )) != NULL);
    return tmp -1;
}
void fileSEND(long connfd, char *fname)
{
	cout<<"In send function File name is "<<fname<<endl;
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
            if (feof(fp))
            {
                
                printf("End of file\n");
                printf("File transfer completed for id: %ld\n", connfd);
                fclose(fp);

            }
            else if (ferror(fp))
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
string filereceive(int sockfd)
{
    FILE *fp;
    string name;
    char recvBuff[1024];
    memset(recvBuff, '0', sizeof(recvBuff));
    char fname[100];
    read(sockfd, fname, 256);
    //strcat(fname,"AK");
    printf("File Name: %s\n",fname);
    name = fname;
    printf("Receiving file...");
     fp = fopen(fname, "ab"); 
        if(NULL == fp)
        {
         printf("Error opening file");
         return "1";
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
            return name;
        }
        if (ferror(fp))
            printf("Error reading\n");
        break;
    }
    }
    fclose(fp);
    
    printf("\nFile OK....Completed\n");
    return name;
}
auth authenticate(int connfd)
{
	char msg[] = "\n\nEnter 1 for login\nEnter 2 for signup";
	send(connfd, msg, strlen(msg), 0); 
	bzero(msg, sizeof(msg));
	memset(msg, 0, sizeof(msg));

	char msg1[1];
	bzero(msg1, sizeof(msg1));
	recv(connfd, msg1, 1, 0);
	cout <<  "Users choice is: "<< msg1[0] << endl;
	//cout<<"hmmm"<<endl;
	if(msg1[0] == '1')
	{
		struct auth cliAuth;	
		char temp[20];
		memset(temp,'\0',20);
		char x[] = "Enter username \n ";
	
		send(connfd, x, strlen(x), 0);
		recv(connfd, temp, sizeof(temp), 0);
	

		string username = temp;
		cliAuth.cliName = username;
		memset(temp,'\0',20);

		char y[] = "Enter Password \n";
		send(connfd, y, strlen(y), 0);
		recv(connfd, temp, 20, 0);

		string password = temp;
		cout<< "Username is "<<username<< " "<< "Password is "<<password<<endl;
		bzero(temp, sizeof(temp)); 
		string user_info = username+","+password;
		
		cout<<"opening file"<<endl;
		fstream myfile("data.txt");
		if(myfile.is_open())
		{
			cout << "checking user_info  "<<user_info<<endl; 
			string line;
			while(getline(myfile,line))
			{
				if(line == user_info)
				{
					//cout<< "login sucsessful"<<endl;
					cliAuth.cliName= username;
					cliAuth.tocken = 0;
					myfile.close();
					return cliAuth;
				}
			}	
			myfile.close();
			return cliAuth;
		}
		else
		{
			cout<<"Error in opening file\n\n\n";
			return cliAuth;
		} 
	}
	else
	{
		//signupCode
		struct auth cliAuth;	
		char temp[20];
		memset(temp,'\0',20);

		char x[] = "Enter username \n ";
		send(connfd, x, strlen(x), 0);

		recv(connfd, temp, sizeof(temp), 0);

		string username = temp;
		cliAuth.cliName = username;
		memset(temp,'\0',20);

		char y[] = "Enter Password \n";
		send(connfd, y, strlen(y), 0);
		recv(connfd, temp, 20, 0);

		string data = username+","+temp;  //temp is password!
		//open file for writing

		std::ofstream outfile;
		outfile.open("data.txt", std::ios_base::app);

		if (!outfile)
		{
			cout << "Error Opening File" << endl;
			return cliAuth;
		}
		string info = data;
		outfile << info << endl;
		outfile.close();
		cliAuth.tocken = 24;
		return cliAuth;
	}
}
bool SlaveAtService = false;
bool AndroidAtService = false;
int Slave_FD;
int Android_fd;
string androidData;
int main(int argc,char *argv[]) 
{
	if(argc<2)
	{
		cout<<"PORT no. is not provided exiting....."<<endl;
		exit(1);
	}
	pthread_t tid;
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1){
		perror("Socket Creation Failed \n");
		exit (1);
	}
	int port = atoi(argv[1]);
	//struct  hostent * server;
	//server = gethostbyname(argv[1]);
	struct sockaddr_in addr;
	addr.sin_addr.s_addr	= INADDR_ANY;
	addr.sin_family		= AF_INET;
	addr.sin_port		= htons(port);	
	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) 
	{
		perror("Bind failed on socket \n");
		exit(1);
	}
	int backlog = 10;
	int connfd;
	int checkSlave;
	int checkAndroid;
	while(SlaveAtService == false)
	{
		if (listen(fd, backlog) == -1) 
		{
			perror("Listen failed on socket\n");
			exit(1);
		}
		struct sockaddr_in cliaddr;
		socklen_t cliaddr_len = sizeof(cliaddr);
	
		connfd = accept(fd, (struct sockaddr *) &cliaddr, &cliaddr_len);
		if (connfd == -1) 
		{
			perror("Accept failed on socket\n");
			exit(1);
		}
		recv(connfd, &checkSlave, sizeof(int), 0);
		if(checkSlave == -9999)
		{
			Slave_FD = connfd;
			SlaveAtService = true;
			cout<<"Slave ready"<<endl;
		}
	}

	while(AndroidAtService == false)
	{
		if (listen(fd, backlog) == -1) 
		{
			perror("Listen failed on socket\n");
			exit(1);
		}
		struct sockaddr_in cliaddr;
		socklen_t cliaddr_len = sizeof(cliaddr);
		connfd = accept(fd, (struct sockaddr *) &cliaddr, &cliaddr_len);
		if (connfd == -1) 
		{
			perror("Accept failed on socket\n");
			exit(1);
		}
		recv(connfd, &checkAndroid, sizeof(int), 0);
		if(checkAndroid >= 50)
		{
			Android_fd = connfd;
			AndroidAtService = true;
			cout<<"Android Device Ready"<<endl;
			pthread_create(&tid, NULL, AndroidDeviceAsClient, (void *)Android_fd);
		}
	}
	if (listen(fd, backlog) == -1) 
	{
		perror("Listen failed on socket\n");
		exit(1);
	}

	while(1)
	{
		struct sockaddr_in cliaddr;
		socklen_t cliaddr_len = sizeof(cliaddr);
		connfd = accept(fd, (struct sockaddr *) &cliaddr, &cliaddr_len);
		if(connfd <= 0)
		{
			perror("Error in accepting");
		}
		pthread_create(&tid, NULL, ClientConnect, (void *)connfd);
	}
	cout<<"Sever Exiting";
	return 0;
	close(fd);
	close(connfd);
}
void *ClientConnect(void *arg)
{
	long connfd = (long)arg;
	char buffer[100];
	recv(connfd, buffer, 100, 0);
	printf("\nServer Received message %s ", buffer);
	bzero(buffer, sizeof(buffer));
	struct auth cliAuth = authenticate(connfd);
	if(cliAuth.tocken == 0)
	{
	 	cout<< cliAuth.cliName<<endl;
	 	cout<< "Login ho gea ha bhai!";
		char msg[] = "\n\n***********************login sucsessful***********************\nEnter 440 to save file\nEnter 450 for retreve file";
		send(connfd, msg, strlen(msg), 0); 
		bzero(msg, sizeof(msg));
		memset(msg, 0, sizeof(msg));
		int temp;
		recv(connfd, &temp, sizeof(int), 0);

		cout << "Users choice is : "<< temp<<endl;;

		bzero(buffer, sizeof(buffer));

		if(temp == 440)
		{
			send(Slave_FD,&temp,sizeof(int),0);
			string fname;
			fname = filereceive(connfd);
			sleep(5);
			cout<<"File recieved: "<<fname <<endl;
			fileSEND(Slave_FD,&fname[0]);

            std::ofstream outfile;
            outfile.open("fileslog.txt", std::ios_base::app);

            if (!outfile)
            {
               cout << "Error Opening File" << endl;
            }
            //string info = fname;
            cout<< "fname "<<fname<<endl;
            outfile << fname << endl;

            outfile.close(); 
            //const char *abc = fname.c_str(); 
            //remove(abc);        
		}
		else
		{
			cout<<"Here in 450  server funtion"<<endl;
			char msg[] = "\nEnter File Name you want to retreve\n";
			send(connfd, msg, strlen(msg), 0); 
		

			FILE* f = fopen("fileslog.txt", "r");

			// Determine file size
			fseek(f, 0, SEEK_END);
			size_t size = ftell(f);
			char* where = new char[1024];
			rewind(f);
			fread(where, sizeof(char), size, f);
			send(connfd, where, strlen(where), 0); 
			delete[] where;
			fclose(f);
			cout<<"getting file name"<<endl;
			bzero(buffer, sizeof(buffer));
			recv(connfd, buffer, 100, 0);
			cout<< "file name recieved is: "<<buffer<<endl;

			// send ready message to slave
			int x = 450;
			cout<<"Requesting File from slave"<<endl;
			send(Slave_FD, &x, sizeof(int), 0);
			sleep(2);
			// send filename to slave
			cout<<"Sending File name to client"<<endl;
			send(Slave_FD, buffer, sizeof(buffer), 0);
			bzero(buffer, sizeof(buffer));
			
			// recieve file from slave 
			cout<<"server is going to recieing file"<<endl;
			string file_name;
			file_name = filereceive(Slave_FD);
			// send file to client
			cout<<"Server is sending file to client"<<endl;
			//const char *xyz = file_name.c_str();
			fileSEND(connfd,&file_name[0]);
			//remove(xyz);

		}
		char z[]="Operation completed";
		send(connfd, z, strlen(z), 0); 
	}
	else if(cliAuth.tocken == 24)
	{
		char msg[] = "\n\n***********************signup sucsessful***********************\nEnter 440 to save file\n";
		send(connfd, msg, strlen(msg), 0); 
		bzero(msg, sizeof(msg));
		memset(msg, 0, sizeof(msg));
		int temp;
		recv(connfd, &temp, sizeof(int), 0);


	}
	else
	{
		char msg[] = "\n\n***********************Login Failed***********************\n";
		send(connfd, msg, strlen(msg), 0); 
		close(connfd);
	}
}
void *AndroidDeviceAsClient(void *arg)
{
	long connfd = (long)arg;
	string temperature,location;
	char buffer[256];
	//cout<<"main phunch gea"<<endl;
	while(1)
	{
		recv(connfd, buffer, 256, 0);
		temperature = buffer;
		bzero(buffer, sizeof(buffer));
		char buffer[256];
		recv(connfd, buffer, 256, 0);
		location = buffer;
		bzero(buffer, sizeof(buffer));
		androidData = "Battery temperature is "+temperature + " and location is " + location;
		cout<<"androidData is\n"<<androidData<<endl;
	}
}