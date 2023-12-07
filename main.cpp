#include <iostream>
#include "Network.h"
#include <string>

using namespace std;

int main(int argc, char *argv[]) {

    //Outputer
    std::ofstream logfile("log.txt");

    if (!logfile.is_open()) {
        std::cerr << "Error: Failed to open log file." << std::endl;
        return 1;
    }

    std::streambuf* original_stdout = std::cout.rdbuf();
    std::cout.rdbuf(logfile.rdbuf());
    //Outputer

    // Instantiate HUBBMNET
    Network* HUBBMNET = new Network();

    // Read from input files
    vector<Client> clients = HUBBMNET->read_clients(argv[1]);
    HUBBMNET->read_routing_tables(clients, argv[2]);
    vector<string> commands = HUBBMNET->read_commands(argv[3]);

    // Get additional parameters from the cmd arguments
    int message_limit = stoi(argv[4]);
    string sender_port = argv[5];
    string receiver_port = argv[6];

    // Run the commands
    HUBBMNET->process_commands(clients, commands, message_limit, sender_port, receiver_port);

    // Delete HUBBMNET
    delete HUBBMNET;

    //Outputer
    std::cout.rdbuf(original_stdout);
    logfile.close();
    //Outputer

    return 0;
}


