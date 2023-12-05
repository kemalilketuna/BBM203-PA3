//
// Created by alperen on 27.09.2023.
//

#include "Client.h"

Client::Client(string const& _id, string const& _ip, string const& _mac) {
    client_id = _id;
    client_ip = _ip;
    client_mac = _mac;
}

ostream &operator<<(ostream &os, const Client &client) {
    os << "client_id: " << client.client_id << " client_ip: " << client.client_ip << " client_mac: "
       << client.client_mac << endl;
    return os;
}

Client::~Client() {
    // Iterate through each stack in the incoming_queue
    while (!incoming_queue.empty()) {
        stack<Packet*>& packetStack = incoming_queue.front();
        // Deallocate each Packet object in the stack
        while (!packetStack.empty()) {
            delete packetStack.top();
            packetStack.pop();
        }
        incoming_queue.pop();
    }

    // Repeat the same process for the outgoing_queue
    while (!outgoing_queue.empty()) {
        stack<Packet*>& packetStack = outgoing_queue.front();
        while (!packetStack.empty()) {
            delete packetStack.top();
            packetStack.pop();
        }
        outgoing_queue.pop();
    }
}