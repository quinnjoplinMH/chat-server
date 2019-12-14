#ifndef CHATROOM_H
#define CHATROOM_H
#include <iostream>
#include <map>
#include <queue>
#include <cstring>
#include <sys/socket.h>
using namespace std;

struct Message {
    int senderSockId;
    string message;
};

class ChatRoom {
    private:
	int id;
	map<int, string> socketUserMap; //Map between socketID and username
	queue<Message> incomingMessages;
	void sendMessage(int sockID, Message message);
    public:
	ChatRoom(int);
        void cycle();
	int getId();
	void addUser(int, string);
	void removeUser(int);
	bool hasMessages();
	void post(Message message); //To send a message to all members of the queue except the sender
	void postFromQueue();
};

#endif
