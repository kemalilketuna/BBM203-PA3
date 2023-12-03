#include "Network.h"
#include <iostream>
#include <sstream>
Network::Network() {

}

void Network::process_commands(vector<Client> &clients, vector<string> &commands, int message_limit,
                      const string &sender_port, const string &receiver_port) {
    // TODO: Execute the commands given as a vector of strings while utilizing the remaining arguments.
    /* Don't use any static variables, assume this method will be called over and over during testing.
     Don't forget to update the necessary member variables after processing each command. For example,
     after the MESSAGE command, the outgoing queue of the sender must have the expected frames ready to send. */
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
