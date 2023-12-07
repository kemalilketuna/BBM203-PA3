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
#include "Log.h"

Network::Network() {

}


void Network::process_commands(vector<Client> &clients, vector<string> &commands, int message_limit,
                      const string &sender_port, const string &receiver_port) {
    int command_count = commands.size();
    for (int i = 0; i < command_count; ++i) {
        string command = commands[i];
        
        // Print command
        cout << string(9 + command.size(), '-') << endl;
        cout << "Command: " <<command << endl;
        cout << string(9 + command.size(), '-') << endl;
        
        if(command.substr(0,7) == "MESSAGE"){
            message_command(clients, message_limit, sender_port, receiver_port, command);
        }
        else if(command.substr(0,15) == "SHOW_FRAME_INFO"){
            show_frame_info_command(clients, command);
        }
        else if(command.substr(0,11) == "SHOW_Q_INFO"){
            show_q_info_command(clients, command);
        }
        else if(command == "SEND"){
            send_command(clients);
        }
        else if(command == "RECEIVE"){
            receive_command(clients);
        }
        else if(command.substr(0,9) == "PRINT_LOG"){
            print_log_command(clients, command);
        }
        else{
            invalid_command();
        }
    }
}

void Network::message_command(vector<Client> &clients, int message_limit, const string sender_port, const string receiver_port, string command){
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
    Client* sender = client_find_id(clients, sender_id);
    Client* receiver = client_find_id(clients, receiver_id);

    // Find the next hop
    string next_hop = sender->routing_table[receiver_id];
    Client* next_hop_client = client_find_id(clients, next_hop);

    // Create frames and push them to the outgoing queue of the sender
    int frame_idx = 0;
    for (int i = 0; i < message_chunks.size(); ++i) {
        frame_idx++;
        stack<Packet*> new_frame;
        ApplicationLayerPacket* app_packet = new ApplicationLayerPacket(0, sender_id, receiver_id, message_chunks[i]);
        TransportLayerPacket* transport_packet = new TransportLayerPacket(1, sender_port, receiver_port);
        NetworkLayerPacket* network_packet = new NetworkLayerPacket(2, sender->client_ip, receiver->client_ip);
        PhysicalLayerPacket* physical_packet = new PhysicalLayerPacket(3, sender->client_mac, next_hop_client->client_mac);
        physical_packet->set_frame_idx(frame_idx);

        new_frame.push(app_packet);
        new_frame.push(transport_packet);
        new_frame.push(network_packet);
        new_frame.push(physical_packet);
        sender->outgoing_queue.push(new_frame);

        // Print the frame
        cout << "Frame #" << frame_idx << endl;
        physical_packet->print();
        network_packet->print();
        transport_packet->print();
        app_packet->print();
        cout << "Message chunk carried: \"" << message_chunks[i] << "\"" << endl;
        cout << "Number of hops so far: 0" << endl;
        cout << "--------" << endl;
    }

    // Create Log
    sender->log_entries.push_back(Log("2023-11-22 20:30:03", message, frame_idx, 0, sender_id, receiver_id, true, ActivityType::MESSAGE_SENT));
}

void Network::show_frame_info_command(vector<Client> &clients, string command){
    string command_name, client_id, in_out, frame_index;
    istringstream iss(command);
    iss >> command_name >> client_id >> in_out >> frame_index;
    
    int frame_idx = stoi(frame_index);
    bool is_in = (in_out == "in");

    // Find the client
    Client* client = client_find_id(clients, client_id);


    // Determine the queue
    queue<stack<Packet*>> temp_queue = is_in ? client->incoming_queue : client->outgoing_queue;


    // Find the frame
    if(frame_idx > temp_queue.size()){
        cout << "No such frame." << endl;
        return;
    }else{
        for (int i = 1; i < frame_idx; ++i) {
            temp_queue.pop();
        }
    }
    
    
    // Get the frame copy
    stack<Packet*> frame = temp_queue.front();
  
    // TODO: Check this part
    if(frame.size() == 4){
        PhysicalLayerPacket* physical_packet = (PhysicalLayerPacket*) frame.top();
        frame.pop();
        NetworkLayerPacket* network_packet = (NetworkLayerPacket*) frame.top();
        frame.pop();
        TransportLayerPacket* transport_packet = (TransportLayerPacket*) frame.top();
        frame.pop();
        ApplicationLayerPacket* app_packet = (ApplicationLayerPacket*) frame.top();
        frame.pop();
        // Print the frame
        cout << "Current Frame #" << frame_idx << " on the " << (is_in ? "incoming" : "outgoing") << " queue of client " << client_id << endl;
        
        // TODO: Print the frame
        cout << "Carried Message: \"" << app_packet->message_data << "\"" << endl;
        
        cout << "Layer 0 info: ";
        app_packet->print();
        cout << "Layer 1 info: ";
        transport_packet->print();
        cout << "Layer 2 info: ";
        network_packet->print();
        cout << "Layer 3 info: ";
        physical_packet->print();
        cout << "Number of hops so far: " << physical_packet->get_hop_count() << endl;
    }
}

void Network::show_q_info_command(vector<Client> &clients, string command){
    string command_name, client_id, in_out, frame_index;
    istringstream iss(command);
    iss >> command_name >> client_id >> in_out;

    bool is_in = (in_out == "in");

    // Find the client
    Client* client = client_find_id(clients, client_id);

    // Determine the queue
    queue<stack<Packet*>> temp_queue = is_in ? client->incoming_queue : client->outgoing_queue;

    // Print the queue info
    cout << "Client " << client_id << " " << (is_in ? "Incoming" : "Outgoing") << " Queue Status" << endl;
    cout << "Current total number of frames: " << temp_queue.size() << endl;
}

void Network::send_command(vector<Client> &clients){
    for (int i = 0; i < clients.size(); ++i) {
        Client *sender = &clients[i];
        if(sender->outgoing_queue.empty()){
            continue;
        }

        while (!sender->outgoing_queue.empty()){
            stack<Packet*> frame = sender->outgoing_queue.front();
            sender->outgoing_queue.pop();
            PhysicalLayerPacket* physical_packet = (PhysicalLayerPacket*) frame.top();
            frame.pop();
            NetworkLayerPacket* network_packet = (NetworkLayerPacket*) frame.top();
            frame.pop();
            TransportLayerPacket* transport_packet = (TransportLayerPacket*) frame.top();
            frame.pop();
            ApplicationLayerPacket* app_packet = (ApplicationLayerPacket*) frame.top();
            frame.pop();

            // Find the next hop
            Client *next_hop_client = client_find_mac(clients, physical_packet->receiver_MAC_address);
            
            // Update the hop count
            physical_packet->increase_hop_count();

            // Push the frame to the incoming queue of the next hop
            stack<Packet*> new_frame;
            new_frame.push(app_packet);
            new_frame.push(transport_packet);
            new_frame.push(network_packet);
            new_frame.push(physical_packet);
            next_hop_client->incoming_queue.push(new_frame);

            // Print the process
            cout << "Client " << sender->client_id << " sending frame #" << physical_packet->get_frame_idx() << " to client " << next_hop_client->client_id << endl;
            physical_packet->print();
            network_packet->print();
            transport_packet->print();
            app_packet->print();
            cout << "Message chunk carried: \"" << app_packet->message_data << "\"" << endl;
            cout << "Number of hops so far: " << physical_packet->get_hop_count() << endl;
            cout << "--------" << endl;
        }
    }
}

void Network::receive_command(vector<Client> &clients){

    for (int i = 0; i < clients.size(); ++i) {
        Client *client = &clients[i];
        if(client->incoming_queue.empty()){
            continue;
        }

        string sender_client_id = "";
        string received_message = "";
        while (!client->incoming_queue.empty()){
            stack <Packet*> frame = client->incoming_queue.front();
            client->incoming_queue.pop();

            PhysicalLayerPacket* physical_packet = (PhysicalLayerPacket*) frame.top();
            frame.pop();
            NetworkLayerPacket* network_packet = (NetworkLayerPacket*) frame.top();
            frame.pop();
            TransportLayerPacket* transport_packet = (TransportLayerPacket*) frame.top();
            frame.pop();
            ApplicationLayerPacket* app_packet = (ApplicationLayerPacket*) frame.top();
            frame.pop();

            Client *previous_hop_client = client_find_mac(clients, physical_packet->sender_MAC_address);

            // Check message received
            if(app_packet->receiver_ID == client->client_id){
                // Message received
                received_message += app_packet->message_data;
                //Client E receiving frame #4 from client D, originating from client C
                cout << "Client " << client->client_id << " receiving frame #" << physical_packet->get_frame_idx() << " from client " << previous_hop_client->client_id << ", originating from client " << app_packet->sender_ID << endl;
                physical_packet->print();
                network_packet->print();
                transport_packet->print();
                app_packet->print();
                cout << "Message chunk carried: \"" << app_packet->message_data << "\"" << endl;
                cout << "Number of hops so far: " << physical_packet->get_hop_count() << endl;
                cout << "--------" << endl;
                
                if(
                    (client->incoming_queue.empty() and received_message.length() != 0) or
                    (!client->incoming_queue.empty() and ((PhysicalLayerPacket*)client->incoming_queue.front().top())->get_frame_idx() < physical_packet->get_frame_idx())
                ){
                    // Create Log
                    client->log_entries.push_back(Log("2023-11-22 20:30:03", received_message, physical_packet->get_frame_idx(), physical_packet->get_hop_count(), app_packet->sender_ID, app_packet->receiver_ID, true, ActivityType::MESSAGE_RECEIVED));

                    //Client E received the message "A few small hops for frames, but a giant leap for this message." from client C.
                    cout << "Client " << client->client_id << " received the message \"" << received_message << "\" from client " << app_packet->sender_ID << "." << endl;
                    cout << "--------" << endl;
                    received_message = "";
                }
                
                delete app_packet;
                delete transport_packet;
                delete network_packet;
                delete physical_packet;
                
            }else{
                //Client B receiving frame #1 from client C, but intended for client E. Forwarding... 
                string next_hop = client->routing_table[app_packet->receiver_ID];
                Client* next_hop_client = client_find_id(clients, next_hop);
                if(next_hop_client == nullptr){
                    int hop_count = physical_packet->get_hop_count();
                    int frame_idx = physical_packet->get_frame_idx();
                    string receiver_id = app_packet->receiver_ID;
                    string sender_id = app_packet->sender_ID;
                    //Client X receiving frame #1 from client C, but intended for client D. Forwarding... 
                    //Error: Unreachable destination. Packets are dropped after 1 hops!
                    cout << "Client " << client->client_id << " receiving frame #" << physical_packet->get_frame_idx() << " from client " << previous_hop_client->client_id << ", but intended for client " << app_packet->receiver_ID << ". Forwarding... " << endl;
                    cout << "Error: Unreachable destination. Packets are dropped after "<< physical_packet->get_hop_count() <<" hops!" << endl;
                    delete app_packet;
                    delete transport_packet;
                    delete network_packet;
                    delete physical_packet;

                    while(!client->incoming_queue.empty() and ((PhysicalLayerPacket*)client->incoming_queue.front().top())->get_frame_idx() > frame_idx){
                        stack <Packet*> frame = client->incoming_queue.front();
                        client->incoming_queue.pop();

                        PhysicalLayerPacket* physical_packet = (PhysicalLayerPacket*) frame.top();
                        frame.pop();
                        NetworkLayerPacket* network_packet = (NetworkLayerPacket*) frame.top();
                        frame.pop();
                        TransportLayerPacket* transport_packet = (TransportLayerPacket*) frame.top();
                        frame.pop();
                        ApplicationLayerPacket* app_packet = (ApplicationLayerPacket*) frame.top();
                        frame.pop();
                        cout << "Client " << client->client_id << " receiving frame #" << physical_packet->get_frame_idx() << " from client " << previous_hop_client->client_id << ", but intended for client " << app_packet->receiver_ID << ". Forwarding... " << endl;
                        cout << "Error: Unreachable destination. Packets are dropped after "<< physical_packet->get_hop_count() <<" hops!" << endl;
                        frame_idx = physical_packet->get_frame_idx();
                        delete app_packet;
                        delete transport_packet;
                        delete network_packet;
                        delete physical_packet;    
                    }

                    // Create Log
                    client->log_entries.push_back(Log("2023-11-22 20:30:03", "", frame_idx, hop_count, sender_id, receiver_id, false, ActivityType::MESSAGE_DROPPED));
                    cout << "--------" << endl;
                }else{
                    int frame_idx = physical_packet->get_frame_idx();
                    //Client D receiving a message from client B, but intended for client E. Forwarding... 
                    cout << "Client " << client->client_id << " receiving a message from client "  << previous_hop_client->client_id << ", but intended for client " << app_packet->receiver_ID << ". Forwarding... " << endl;
                    // Frame #1 MAC address change: New sender MAC BBBBBBBBBB, new receiver MAC DDDDDDDDDD
                    cout << "Frame #" << physical_packet->get_frame_idx() << " MAC address change: New sender MAC " << client->client_mac << ", new receiver MAC " << next_hop_client->client_mac << endl;
                    physical_packet->sender_MAC_address = client->client_mac;
                    physical_packet->receiver_MAC_address = next_hop_client->client_mac;
                    stack<Packet*> new_frame;
                    new_frame.push(app_packet);
                    new_frame.push(transport_packet);
                    new_frame.push(network_packet);
                    new_frame.push(physical_packet);
                    client->outgoing_queue.push(new_frame);    
                    
                    while(!client->incoming_queue.empty() and ((PhysicalLayerPacket*)client->incoming_queue.front().top())->get_frame_idx() > physical_packet->get_frame_idx()){
                        stack <Packet*> frame = client->incoming_queue.front();
                        client->incoming_queue.pop();

                        PhysicalLayerPacket* physical_packet = (PhysicalLayerPacket*) frame.top();
                        frame.pop();
                        NetworkLayerPacket* network_packet = (NetworkLayerPacket*) frame.top();
                        frame.pop();
                        TransportLayerPacket* transport_packet = (TransportLayerPacket*) frame.top();
                        frame.pop();
                        ApplicationLayerPacket* app_packet = (ApplicationLayerPacket*) frame.top();
                        frame.pop();
                        frame_idx = physical_packet->get_frame_idx();
                        cout << "Frame #" << physical_packet->get_frame_idx() << " MAC address change: New sender MAC " << client->client_mac << ", new receiver MAC " << next_hop_client->client_mac << endl;
                        physical_packet->sender_MAC_address = client->client_mac;
                        physical_packet->receiver_MAC_address = next_hop_client->client_mac;
                        stack<Packet*> new_frame;
                        new_frame.push(app_packet);
                        new_frame.push(transport_packet);
                        new_frame.push(network_packet);
                        new_frame.push(physical_packet);
                        client->outgoing_queue.push(new_frame);    
                    }
                    cout << "--------" << endl;
                    
                    // Create Message Forwarded Log
                    client->log_entries.push_back(Log("2023-11-22 20:30:03", "", frame_idx, physical_packet->get_hop_count(), app_packet->sender_ID, app_packet->receiver_ID, true, ActivityType::MESSAGE_FORWARDED));
                }
            }
        }
    }      
}

void Network::print_log_command(vector<Client> &clients, string command){
    // timestamp 2023-11-22 20:30:03
    string command_name, client_id;
    istringstream iss(command);
    iss >> command_name >> client_id;
    Client *client = client_find_id(clients, client_id);
    vector<Log>& logs = client->log_entries;

    if(logs.empty()){
        return;
    }

    cout << "Client " << client_id << " Logs:" << endl;
    
    for (int i = 0; i < logs.size(); ++i) {
        Log log = logs[i];
        cout << "--------------" << endl;
        string activity_type = "";
        switch (log.activity_type) {
            case ActivityType::MESSAGE_RECEIVED:
                activity_type = "Message Received";
                break;
            case ActivityType::MESSAGE_FORWARDED:
                activity_type = "Message Forwarded";
                break;
            case ActivityType::MESSAGE_SENT:
                activity_type = "Message Sent";
                break;
            case ActivityType::MESSAGE_DROPPED:
                activity_type = "Message Dropped";
                break;
        }
        
        cout << "Log Entry #" << i+1 << ":" <<endl;
        cout << "Activity: " << activity_type << endl;
        cout << "Timestamp: " << log.timestamp << endl;
        cout << "Number of frames: " << log.number_of_frames << endl;
        cout << "Number of hops: " << log.number_of_hops << endl;
        cout << "Sender ID: " << log.sender_id << endl;
        cout << "Receiver ID: " << log.receiver_id << endl;
        cout << "Success: " << (log.success_status ? "Yes" : "No") << endl;
        if(log.activity_type == ActivityType::MESSAGE_RECEIVED){
            cout << "Message: \"" << log.message_content << "\"" << endl;
        }
        if(log.activity_type == ActivityType::MESSAGE_SENT){
            cout << "Message: \"" << log.message_content << "\"" << endl;
        }
    }
}

void Network::invalid_command(){
    cout << "Invalid command." << endl;
}

Client *Network::client_find_mac(vector<Client> &clients, string client_mac) {
    for (int i = 0; i < clients.size(); ++i) {
        if(clients[i].client_mac == client_mac){
            return &clients[i];
        }
    }
    return nullptr;
}

Client *Network::client_find_id(vector<Client> &clients, string client_id) {
    for (int i = 0; i < clients.size(); ++i) {
        if(clients[i].client_id == client_id){
            return &clients[i];
        }
    }
    return nullptr;
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
