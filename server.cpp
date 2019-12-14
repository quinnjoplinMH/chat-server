#include "ChatRoom.h"
#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <utility>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <thread>

//MYSQL dependencies
#include <cppconn/driver.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <mysql_error.h>

using namespace std;
using namespace sql::mysql;



class Server {
  private:
    map<string, string> usernamePasswordMap;
    map<int, ChatRoom*> chatRoomMap; //Map between ChatRoom.id and the ChatRoom object
   // map<int, thread> threadMap; //Map between ChatRoom.pid/PID of thread and the Thread object
  public:
    void *handle_connection(void* clientSd);
    void addUser(string username, string password);
};

struct new_connection {
    Server server;
    int clientSd;
};

void* handle_connection(void* new_conn) {
    ((new_connection*)new_conn)->server.handle_connection(&((new_connection*)new_conn)->clientSd);
}

void* cycle(void* room) {
    ((ChatRoom*)room)->cycle();
}

void Server::addUser(string username, string password) {
    usernamePasswordMap.insert(make_pair(username, password));
}

void* Server::handle_connection(void* clientSd) {
    char cred[500];
    memset(&cred, 0, sizeof(cred));
    int bytesRead = recv((intptr_t)clientSd, (char*)&cred, sizeof(cred), 0);
    string username;
    string password;
    string roomIdString;
    int roomId;
    int tmp = 0;
    for (int i = 0; i < 500 && tmp < 3; i++) {
        if (cred[i] != ' ') {
            switch (tmp) {
                case 0: username += cred[i]; break;
                case 1: password += cred[i]; break;
                case 2: roomIdString += cred[i]; break;
            }
        } else {
            tmp++;
        }
    }
    roomId = stoi(roomIdString);
    auto it = usernamePasswordMap.find(username);
    if (it != usernamePasswordMap.end() && password == it->second) {
        if (chatRoomMap.empty()) {
	    chatRoomMap[roomId]->addUser((intptr_t)clientSd, username);
	} else {
	    ChatRoom * newRoom = new ChatRoom(roomId);
	    newRoom->addUser((intptr_t)clientSd, username);
	    chatRoomMap.insert({roomId, newRoom});
   	    pthread_t cycleThread;
	    pthread_create(&cycleThread, NULL, cycle, &newRoom);
        }
    } else {
      string message = "Invalid Credentials!";
      send((intptr_t)clientSd, (char*)&message, message.length(), 0);
    }
}

Server server;

int main(int argc, char *argv[])
{
    server.addUser("username", "password");
    //for the server, we only need to specify a port number
    if(argc != 2)
    {
        cerr << "Usage: port" << endl;
        exit(0);
    }
    //grab the port number
    int port = atoi(argv[1]);
    //buffer to send and receive messages with
    char msg[1500];

    //setup a socket and connection tools
    sockaddr_in servAddr;
    bzero((char*)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);

    //open stream oriented socket with internet address
    //also keep track of the socket descriptor
    int serverSd = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSd < 0)
    {
        cerr << "Error establishing the server socket" << endl;
        exit(0);
    }
    //bind the socket to its local address
    int bindStatus = bind(serverSd, (struct sockaddr*) &servAddr, 
        sizeof(servAddr));
    if(bindStatus < 0)
    {
        cerr << "Error binding socket to local address" << endl;
        exit(0);
    }
    while (1) {
        cout << "Waiting for a client to connect..." << endl;
        //listen for up to 5 requests at a time
        listen(serverSd, 5);
        //receive a request from client using accept
        //we need a new address to connect with the client
        sockaddr_in newSockAddr;
        socklen_t newSockAddrSize = sizeof(newSockAddr);
        int newSd = accept(serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);
        if(newSd < 0)
        {
            cerr << "Error accepting request from client!" << endl;
            exit(1);
        }
        cout << "Connected with client!" << endl;
        pthread_t connectionHandler;
	new_connection conn;
	conn.server = server;
	conn.clientSd = newSd;
	void* newConn = &conn;
	pthread_create(&connectionHandler, NULL, handle_connection, newConn);
	int status = 1;
	if (status == -1) {
	    cout << "Failed to create new proccess" << endl;
	}
    }
    cout << "Connection closed..." << endl;
    return 0;
}
