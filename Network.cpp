#include <iostream>
#include <sstream>
#include <string>
#include <stack>
#include <queue>

#include "Network.h"
#include "Client.h"
#include "ApplicationLayerPacket.h"
#include "TransportLayerPacket.h"
#include "NetworkLayerPacket.h"
#include "PhysicalLayerPacket.h"

Network::Network() {

}


void Network::process_commands(vector<Client> &clients, vector<string> &commands, int message_limit,
                      const string &sender_port, const string &receiver_port) {
    // TODO: Execute the commands given as a vector of strings while utilizing the remaining arguments.
    /* Don't use any static variables, assume this method will be called over and over during testing.
     Don't forget to update the necessary member variables after processing each command. For example,
     after the MESSAGE command, the outgoing queue of the sender must have the expected frames ready to send. */
    int command_count = commands.size();
    for (int i = 0; i < command_count; ++i) {
        string command = commands[i];
        if(command.substr(0,7) == "MESSAGE"){
            message_command(clients, message_limit, sender_port, receiver_port, command);
        }
        else if(command.substr(0,15) == "SHOW_FRAME_INFO"){
            show_frame_info_command();
        }
        else if(command.substr(0,11) == "SHOW_Q_INFO"){
            show_q_info_command();
        }
        else if(command == "SEND"){
            send_command();
        }
        else if(command == "RECEIVE"){
            receive_command();
        }
        else if(command.substr(0,9) == "PRINT_LOG"){
            print_log_command();
        }
        else{
            invalid_command();
        }
    }
}

void Network::message_command(vector<Client> &clients, int message_limit, const string sender_port, const string receiver_port, string command){
    // Print command
    cout << string(9 + command.size(), '-') << endl;
    cout << "Command: " <<command << endl;
    cout << string(9 + command.size(), '-') << endl;

    // Split the message
    std::istringstream iss(command);
    string command_name, sender_id, receiver_id, message;
    iss >> command_name >> sender_id >> receiver_id;

    // Extract the message
    getline(iss, message);
    message = message.substr(2, message.length()-3);

    // Print the message
    cout << "Message to be sent: \"" << message << "\"\n" << endl;

    // Split the message into frames
    vector<string> message_chunks;
    int message_length = message.length();
    int numChunks = (message_length + message_limit - 1) / message_limit;
    for (int i = 0; i < numChunks; ++i) {
        message_chunks.push_back(message.substr(i*message_limit, message_limit));
    }

    // Find the sender and receiver clients
    Client* sender = nullptr;
    Client* receiver = nullptr;
    Client* next_hop_client = nullptr;
    for (int i = 0; i < clients.size(); ++i) {
        if(clients[i].client_id == sender_id){
            sender = &clients[i];
        }
        if(clients[i].client_id == receiver_id){
            receiver = &clients[i];
        }
    }

    // Find the next hop
    string next_hop = sender->routing_table[receiver_id];
    for (int i = 0; i < clients.size(); ++i) {
        if(clients[i].client_id == next_hop){
            next_hop_client = &clients[i];
        }
    }

    // Create frames and push them to the outgoing queue of the sender
    int frame_idx = 0;
    for (int i = 0; i < message_chunks.size(); ++i) {
        stack<Packet*> new_frame;
        ApplicationLayerPacket* app_packet = new ApplicationLayerPacket(0, sender_id, receiver_id, message_chunks[i]);
        TransportLayerPacket* transport_packet = new TransportLayerPacket(1, sender_port, receiver_port);
        NetworkLayerPacket* network_packet = new NetworkLayerPacket(2, sender->client_ip, receiver->client_ip);
        PhysicalLayerPacket* physical_packet = new PhysicalLayerPacket(3, sender->client_mac, next_hop_client->client_mac);
        
        new_frame.push(app_packet);
        new_frame.push(transport_packet);
        new_frame.push(network_packet);
        new_frame.push(physical_packet);
        sender->outgoing_queue.push(new_frame);

        // Print the frame
        frame_idx++;
        cout << "Frame #" << frame_idx << endl;
        physical_packet->print();
        network_packet->print();
        transport_packet->print();
        app_packet->print();
        cout << "Message chunk carried: \"" << message_chunks[i] << "\"" << endl;
        cout << "Number of hops so far: 0" << endl;
        cout << "--------" << endl;
    }
}

void Network::show_frame_info_command(){
    cout << "SHOW_FRAME_INFO" << endl;
}

void Network::show_q_info_command(){
    cout << "SHOW_Q_INFO" << endl;
}

void Network::send_command(){
    cout << "SEND" << endl;
}

void Network::receive_command(){
    cout << "RECEIVE" << endl;
}

void Network::print_log_command(){
    // timestamp 2023-11-22 20:30:03
    cout << "PRINT_LOG" << endl;
}

void Network::invalid_command(){
    cout << "Invalid command" << endl;
}

vector<Client> Network::read_clients(const string &filename) {
    vector<Client> clients;
    int client_count;
    // open filename
    ifstream file(filename);
    // read client count
    file >> client_count;
    // read clients
    for (int i = 0; i < client_count; ++i) {
        string client_id, client_ip, client_mac;
        file >> client_id >> client_ip >> client_mac;
        clients.push_back(Client(client_id, client_ip, client_mac));
    }
    // close filename
    file.close();
    return clients;
}

void Network::read_routing_tables(vector<Client> &clients, const string &filename) {
    int client_count = clients.size();
    // open filename
    ifstream file(filename);
    int client_index = 0;
    // read routing tables
    string line;
    while (getline(file, line)) {
        if(line[0]=='-'){
            client_index++;
            continue;
        }
        istringstream iss(line);
        string target_des, next_hop;
        iss >> target_des >> next_hop;
        clients[client_index].routing_table[target_des] = next_hop;

    }

    // close filename
    file.close();
}

// Returns a list of token lists for each command
vector<string> Network::read_commands(const string &filename) {
    vector<string> commands;
    // TODO: Read commands from the given input file and return them as a vector of strings.
    
    // open filename
    ifstream file(filename);
    
    // read command count
    int command_count;
    file >> command_count;

    // read commands line by line
    string line;
    getline(file, line);
    for (int i = 0; i < command_count; ++i) {
        getline(file, line);
        commands.push_back(line);
    }

    // close filename
    file.close();
    
    return commands;
}

Network::~Network() {
    // TODO: Free any dynamically allocated memory if necessary.
}
