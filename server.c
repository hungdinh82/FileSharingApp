#include <stdio.h>          /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>

#include "protocol.h"
#include "validate.h"
#include "status.h"
#include "path.h"
#include "authenticate.h"


#define PORT 5550   /* Port that will be opened */ 
#define BACKLOG 3   /* Number of allowed connections */
#define MAX_SIZE 10e6 * 100
#define MAX_LIST_PATH 2048
#define BUFF_SEND 1024
#define PRIVATE_KEY 256
pthread_mutex_t lock;
int requestId = 1;
char fileRepository[100];

Client onlineClient[1000];

/*
* Init Array Client
* Set default value for Online Client
*/
void initArrayClient() {
	int i;
	for(i = 0; i < 1000; i++) {
		//onlineClient[i].requestId = 0;    //set default value 0 for requestId
		onlineClient[i].uploadSuccess = 0; //set default value 0 for uploadSuccess
	}
}

int numberElementsInArray(char** temp) {
	int i;
	for (i = 0; *(temp + i); i++)
    {
        // count number elements in array
    }
    return i;
}

void printListOnlineClient() {
	int i;
	for (i = 0; i < 1000; i++) {
		if(onlineClient[i].requestId > 0) {
			printf("\n---ConnSock---: %d\n", onlineClient[i].connSock);
			printf("---RequestId---: %d\n", onlineClient[i].requestId);
			printf("---Username---: %s\n", onlineClient[i].username);
		}
	}
}

//find Avaiable Position in Array Client
int findAvaiableElementInArrayClient() {
	int i;
	for (i = 0; i < 1000; i++) {
		if(onlineClient[i].requestId == 0) {	// avaiable position is position where requestId = 0
			return i;
		}
	}
	return -1; // if not have avaiable element
}

int findClient(int requestId) {
	int i;
	for (i = 0; i < 1000; i++) {
		if(onlineClient[i].requestId == requestId) {
			return i;
		}
	}
	return -1;
}

int findClientByUsername(char* username) {
	int i;
	for (i = 0; i < 1000; i++) {
		if(!strcmp(onlineClient[i].username, username) && (onlineClient[i].requestId > 0)) {
			return i;
		}
	}
	return -1;
}

// set client into online client array
void setClient(int i, int requestId, char* username, int connSock) {
	if(i >= 0) {
		onlineClient[i].requestId = requestId;
		strcpy(onlineClient[i].username, username); //set username for online client
		onlineClient[i].connSock = connSock;	
	} else {
		printf("Full Client, Service not avaiable!!\n"); //If array client full
	}
}

void increaseRequestId() {
	pthread_mutex_lock(&lock); //use mutex for increase shared data requestId
	requestId++;
	pthread_mutex_unlock(&lock);
}

void decreaseRequestId() {
	pthread_mutex_lock(&lock); //use mutex for increase shared data requestId
	requestId--;
	pthread_mutex_unlock(&lock);
}
/*
* handle find or create folder as user
* @param username
* @return void
*/
void createFolder(char* path) {
	struct stat st = {0};
	if (stat(path, &st) == -1) {
	    mkdir(path, 0700);
	}
}

void handleCreateFolder(Message recvMess){
	createFolder(recvMess.payload);
}

void handleDeleteFolder(Message recvMess){
	remove_dir(recvMess.payload);
}

void handleDeleteFile(Message recvMess){
	remove(recvMess.payload);
}

void handleDirectory(Message recvMess, int connSock) {
	//printMess(recvMess);
	char listFolder[MAX_LIST_PATH];
	char listFile[MAX_LIST_PATH];
	char path[100];
	memset(path,'\0',sizeof(path));
	memset(listFolder,'\0',sizeof(listFolder));
	memset(listFile,'\0',sizeof(listFile));
	int i = findClient(recvMess.requestId);
	strcat(path,"./");
	strcat(path,onlineClient[i].username);
	getListFolder(path,listFolder);
	getListFile(path,listFile);
	Message msg1,msg2;
	//sent list folder
	strcpy(msg1.payload,listFolder);
	msg1.requestId = recvMess.requestId;
	msg1.length = strlen(msg1.payload);
	sendMessage(onlineClient[i].connSock, msg1);
	//send list file
	msg2.type = TYPE_REQUEST_DIRECTORY;
	strcpy(msg2.payload,listFile);
	msg2.requestId = recvMess.requestId;
	msg2.length = strlen(msg2.payload);
	sendMessage(onlineClient[i].connSock, msg2);
}

void handleRequestDownload(Message recvMsg, int connSock) { 
	Message sendMsg;
	Message msg;
	FILE* fptr;
	char fullPath[50];
	strcpy(fullPath,recvMsg.payload);
	int i = findClient(recvMsg.requestId);
	if ((fptr = fopen(fullPath, "rb+")) == NULL){
        printf("Error: File not found\n");
        sendMsg.type = TYPE_ERROR;
		sendMsg.length = 0;
		sendMessage(onlineClient[i].connSock,sendMsg);
		return;
    }
    else {
		sendMsg.type = TYPE_OK;
		sendMsg.length = 0;
		sendMessage(onlineClient[i].connSock,sendMsg);
		receiveMessage(onlineClient[i].connSock,&msg);
		if(msg.type!=TYPE_CANCEL){
			long filelen;
			fseek(fptr, 0, SEEK_END);          // Jump to the end of the file
			filelen = ftell(fptr);             // Get the current byte offset in the file       
			rewind(fptr);    // pointer to start of file
			//int check = 1;
			int sumByte = 0;
			while(!feof(fptr)) {
				int numberByteSend = PAYLOAD_SIZE;
				if((sumByte + PAYLOAD_SIZE) > filelen) {// if over file size
					numberByteSend = filelen - sumByte; 
				}
				char* buffer = (char *) malloc((numberByteSend) * sizeof(char));
				fread(buffer, numberByteSend, 1, fptr); // read buffer with size 
				memcpy(sendMsg.payload, buffer, numberByteSend);
				sendMsg.length = numberByteSend;
				sumByte += numberByteSend; //increase byte send
				//printf("sumByte: %d\n", sumByte);
				if(sendMessage(onlineClient[i].connSock, sendMsg) <= 0) { 
					printf("Connection closed!\n");
					//check = 0;
					break;
				}
				free(buffer);
				if(sumByte >= filelen) {
					break;
				}
			}
			sendMsg.length = 0;
			sendMessage(onlineClient[i].connSock, sendMsg);
		}
    }

}

/*
* remove one file
* @param filename
* @return void
*/
void removeFile(char* fileName) {
	// remove file
    if (remove(fileName) != 0)
    {
        perror("Following error occurred\n");
    }
}

/*
* receive file from client and save
* @param filename, path
* @return void
*/
void handleUploadFile(Message recvMsg, int connSock) {
	FILE *fptr;
	//char fileName[100];
	char fullPath[100];
	Message sendMsg;
	strcpy(fullPath,recvMsg.payload);
	int i = findClient(recvMsg.requestId);

	//printf("FULL PATH: %s and %s",fullPath,recvMsg.payload);
	if(fopen(fullPath,"r")!=NULL ) { // check if file exist
		sendMsg.type = TYPE_ERROR;
		strcpy(sendMsg.payload,"\nFile name already exists");
		sendMsg.length = strlen(sendMsg.payload);
		sendMsg.requestId = recvMsg.requestId;
		sendMessage(onlineClient[i].connSock,sendMsg);
		return;
	}
	else {
		sendMsg.type = TYPE_OK;
		sendMsg.length = 0;
		sendMsg.requestId = recvMsg.requestId;
		sendMessage(onlineClient[i].connSock,sendMsg);
		fptr = fopen(fullPath,"w+");
		while(1) {
			receiveMessage(connSock, &recvMsg);
			if(recvMsg.type == TYPE_ERROR) {
				fclose(fptr);
				removeFile(fullPath);
			}
			if(recvMsg.length > 0) { 
				fwrite(recvMsg.payload, recvMsg.length, 1, fptr);
			} else {
				break;
			}
		}
		fclose(fptr);
		strcpy(sendMsg.payload,"Upload Successful!");
		sendMsg.length = strlen(sendMsg.payload);
		sendMessage(onlineClient[i].connSock,sendMsg);
	}
}

/*
* handle get path of file to folder of user
* @param filename, fullpath
* @return void
*/
void getFullPath(char* fileName, char* fullPath) {
	sprintf(fullPath, "%s/%s", fileRepository, fileName);
}

void handleLogin(Message mess, int connSock) {

	char** temp = str_split(mess.payload, '\n'); // handle payload, divide payload to array string split by '\n'
	StatusCode loginCode;
	//User* curUser = NULL;
	if(numberElementsInArray(temp) == 3) {

		char** userStr = str_split(temp[1], ' '); // get username'
		char** passStr = str_split(temp[2], ' '); // get password
		if((numberElementsInArray(userStr) == 2) && (numberElementsInArray(passStr) == 2)) { //check payload structure valid with two parameters
			if(!(strcmp(userStr[0], COMMAND_USER) || strcmp(passStr[0], COMMAND_PASSWORD))) { //check payload structure valid with two parameters

				char username[30];
				char password[20];
				strcpy(username, userStr[1]);
				strcpy(password, passStr[1]);
				if(validateUsername(username) && validatePassword(password)) { // check username and password are valid
					loginCode = login(username, password); // login with username and password 
					if(loginCode != LOGIN_SUCCESS)
						mess.type = TYPE_ERROR;
					else{
						if(mess.requestId == 0) {
							mess.requestId = requestId;
							increaseRequestId();
							int i = findAvaiableElementInArrayClient();
							setClient(i, mess.requestId, username, connSock); // when user login success set user to online client
							printf("Login ID:%d, rqID: %d, connsock: %d\n",i,mess.requestId,connSock);
							createFolder(username);
						}
					}
				} else {
					mess.type = TYPE_ERROR;
					loginCode = USERNAME_OR_PASSWORD_INVALID; //set login code
				}
			}
			else{
				loginCode = COMMAND_INVALID;
				mess.type=TYPE_ERROR;
			}
		}
		else{
			loginCode = COMMAND_INVALID;
			mess.type=TYPE_ERROR;
		}
	}
	else {
		mess.type=TYPE_ERROR;
		loginCode = COMMAND_INVALID;
	}
	sendWithCode(mess, loginCode, connSock);
}

void handleRegister(Message mess, int connSock){
	char** temp = str_split(mess.payload, '\n');
	StatusCode registerCode;
	if(numberElementsInArray(temp) == 3) {
		char** userStr = str_split(temp[1], ' ');
		char** passStr = str_split(temp[2], ' ');
		if((numberElementsInArray(userStr) == 2) && (numberElementsInArray(passStr) == 2)) {
			if(!(strcmp(userStr[0], COMMAND_USER) || strcmp(passStr[0], COMMAND_PASSWORD))) {
				char username[30];
				char password[20];
				strcpy(username, userStr[1]);
				strcpy(password, passStr[1]);
				if(validateUsername(username) && validatePassword(password)) {
					registerCode = registerUser(username, password);
					if(registerCode != REGISTER_SUCCESS)
						mess.type=TYPE_ERROR;
					else {
						if(mess.requestId == 0) {
							mess.requestId = requestId;
							increaseRequestId();
							int i = findAvaiableElementInArrayClient();
							setClient(i, mess.requestId, username, connSock);
							createFolder(username);
						}
					}
				} else {
					mess.type = TYPE_ERROR;
					registerCode = USERNAME_OR_PASSWORD_INVALID;
				}
			}
			else{
				registerCode = COMMAND_INVALID;
				mess.type=TYPE_ERROR;
			}
		}
		else{
			registerCode = COMMAND_INVALID;
			mess.type=TYPE_ERROR;
		}
	}
	else {
		mess.type=TYPE_ERROR;
		registerCode = COMMAND_INVALID;
		printf("Fails on handle Register!!");
	}
	sendWithCode(mess, registerCode, connSock);
}

void handleLogout(Message mess, int connSock){
	char** temp = str_split(mess.payload, '\n');
	StatusCode logoutCode;
	if(numberElementsInArray(temp) != 2) {
		mess.type = TYPE_ERROR;
		logoutCode = COMMAND_INVALID;
		printf("Fails on handle logout!!");
	}
	else{
		logoutCode = logoutUser(temp[1]);
		if(logoutCode == LOGOUT_SUCCESS) {
			int i = findClient(mess.requestId);
			if(i >= 0) {
				onlineClient[i].requestId = 0;
				onlineClient[i].username[0] = '\0';
				onlineClient[i].connSock=0;
				mess.requestId=0;
				decreaseRequestId();

			}
		}
	}
	sendWithCode(mess, logoutCode, connSock);
}

void handleAuthenticateRequest(Message mess, int connSock) {
	char* payloadHeader;
	char temp[PAYLOAD_SIZE];
	strcpy(temp, mess.payload);
	payloadHeader = getHeaderOfPayload(temp);
	if(!strcmp(payloadHeader, LOGIN_CODE)) {
		handleLogin(mess, connSock);
	} else if (!strcmp(payloadHeader, REGISTER_CODE)) {
		handleRegister(mess, connSock);

	} else if(!strcmp(payloadHeader, LOGOUT_CODE)) {
		handleLogout(mess, connSock);
	}
}

//Handler Request from Client
void* client_handler(void* conn_sock) {
	int connSock;
	connSock = *((int *) conn_sock);
	Message recvMess;
	pthread_detach(pthread_self());
	while(1) {
		//receives message from client
		if(receiveMessage(connSock, &recvMess) < 0) {
			if(recvMess.requestId > 0) {
				int i = findClient(recvMess.requestId);
				if(i >= 0) {
					onlineClient[i].requestId = 0;
					logoutUser(onlineClient[i].username);
				}
			}
			break;
		}
		
		//blocking
		switch(recvMess.type) {
			case TYPE_AUTHENTICATE: 
				handleAuthenticateRequest(recvMess, connSock);
				//printListOnlineClient();
				break;
			case TYPE_REQUEST_DIRECTORY:
				handleDirectory(recvMess, connSock);
				//return NULL;
				break;
			case TYPE_REQUEST_DOWNLOAD: 
				handleRequestDownload(recvMess, connSock);
				break;
			case TYPE_UPLOAD_FILE: 
				handleUploadFile(recvMess, connSock);
				break;
			case TYPE_CREATE_FOLDER:
				handleCreateFolder(recvMess); 
				break;
			case TYPE_DELETE_FOLDER:
				handleDeleteFolder(recvMess);
				break;
			case TYPE_DELETE_FILE:
				handleDeleteFile(recvMess);
			default: break;
		}

	}

	return NULL;
}

int main(int argc, char **argv)
{
 	int port_number;
 	int listen_sock, conn_sock; /* file descriptors */
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client; /* client's address information */
	int sin_size;
	pthread_t tid;
		
 	if(argc != 2) {
 		perror(" Error Parameter! Please input only port number\n ");
 		exit(0);
 	}
 	if((port_number = atoi(argv[1])) == 0) {
 		perror(" Please input correct port number\n");
 		exit(0);
 	}
 	if(!validPortNumber(port_number)) {
 		perror("Invalid Port Number!\n");
 		exit(0);
 	}
	//Step 1: Construct socket	
	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){  /* calls socket() */
		perror("\nError: ");
		return 0;
	}
	//Step 2: Bind address to socket	
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;         
	server.sin_port = htons(port_number);   
	server.sin_addr.s_addr = htonl(INADDR_ANY);  /* INADDR_ANY puts your IP address automatically */   
	if(bind(listen_sock, (struct sockaddr*)&server, sizeof(server)) == -1){ /* calls bind() */
		perror("\nError: ");
		return 0;
	}     
	
	//Step 3: Listen request from client
	if(listen(listen_sock, BACKLOG) == -1){  /* calls listen() */
		perror("\nError: ");
		return 0;
	}
	printf("Server start...\n");
	initArrayClient();
	readFile();
	printList();
	//Step 4: Communicate with client
	while(1) {
		//accept request
		sin_size = sizeof(struct sockaddr_in);
		if ((conn_sock = accept(listen_sock,( struct sockaddr *)&client, (unsigned int*)&sin_size)) == -1) 
			perror("\nError: ");
		printf("You got a connection from %s\n", inet_ntoa(client.sin_addr) ); /* prints client's IP */
		if (pthread_mutex_init(&lock, NULL) != 0) 
	    { 
	        printf("\n mutex init has failed\n"); 
	        return 1; 
	    } 
		//start conversation
		pthread_create(&tid, NULL, &client_handler, &conn_sock);	
	}
	
	close(listen_sock);
	return 0;
}