#include "ChatRoom.h"
#include <iostream>
#include <map>
#include <queue>
#include <cstring>
#include <sys/socket.h>
using namespace std;

// map<int, string> socketUserMap; //Map between socketID and username
// queue<Message> incomingMessages;

ChatRoom::ChatRoom(int id) {
    this->id = id;
}

void ChatRoom::cycle() {
    while (!socketUserMap.empty()) {
	postFromQueue();
    }
}

int ChatRoom::getId() {
    return id;
}

void ChatRoom::addUser(int sockId, string username) {
    socketUserMap.insert({sockId, username});
    Message message;
    message.message = username;
    message.message += " has joined the chat.";
    post(message);
}

void ChatRoom::removeUser(int sockId) {
    auto itr = socketUserMap.find(sockId);
    if (itr != socketUserMap.end()) {
	Message message;
	message.message = socketUserMap[sockId];
	message.message += " has left the chat.";
	post(message);
        socketUserMap.erase(sockId);
    }
}

void ChatRoom::sendMessage(int sockId, Message message){
    send(sockId,(char*)&message.message, message.message.length(), 0);
    cout << "Message sent to " << sockId << " from " << socketUserMap.at(message.senderSockId)
      << endl;
}

bool ChatRoom::hasMessages(){
    return !incomingMessages.empty();
}

//To send a message to all members of the chat room
void ChatRoom::post(Message message) {
    for (auto i = socketUserMap.begin(); i != socketUserMap.end(); i++) {
	sendMessage(i->first, message);
    }
}

void ChatRoom::postFromQueue() {
    if (hasMessages()) {
	Message messageCont = incomingMessages.front();
	incomingMessages.pop();
	if (strcmp((char*)&messageCont.message, "exit")) {
	    removeUser(messageCont.senderSockId);
	} else {
	    post(messageCont);
	}
    }
}
