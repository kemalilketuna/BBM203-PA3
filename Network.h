#ifndef NETWORK_H
#define NETWORK_H

#include <vector>
#include <iostream>
#include "Packet.h"
#include "Client.h"
#include <string>

using namespace std;

class Network {
public:
    Network();
    ~Network();

    // Executes commands given as a vector of strings while utilizing the remaining arguments.
    void process_commands(vector<Client> &clients, vector<string> &commands, int message_limit, const string &sender_port,
                     const string &receiver_port);

    // Initialize the network from the input files.
    vector<Client> read_clients(string const &filename);
    void read_routing_tables(vector<Client> & clients, string const &filename);
    vector<string> read_commands(const string &filename); 
private:
    void message_command(vector<Client> &clients, int message_limit, const string sender_port, const string receiver_port, string command);
    void show_frame_info_command(vector<Client> &clients, string command);
    void show_q_info_command(vector<Client> &clients, string command);
    void send_command(vector<Client> &clients);
    void receive_command(vector<Client> &clients);
    void print_log_command(vector<Client> &clients, string command);
    void invalid_command();
    Client* client_find_id(vector<Client> &clients, string client_id);
    Client* client_find_mac(vector<Client> &clients, string client_mac);
};

#endif  // NETWORK_H
