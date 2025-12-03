#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <fmt/format.h>
#include <getopt.h>
#include "http/handler.h"
#include "storage/sqlite_storage.h"
#include "service/contract_service.h"

using namespace std;

struct Config {
    int port = 8080;
    string db_path = "./data/contracts.db";
};

void print_usage(const char* program_name) {
    cout << "Usage: " << program_name << " [OPTIONS]" << endl;
    cout << "Options:" << endl;
    cout << "  -p, --port PORT    Set the server port (default: 8080)" << endl;
    cout << "  -d, --db PATH      Set the database file path (default: ./data/contracts.db)" << endl;
    cout << "  -h, --help         Display this help message" << endl;
}

Config parse_command_line(int argc, char* argv[]) {
    Config config;
    
    static struct option long_options[] = {
        {"port", required_argument, nullptr, 'p'},
        {"db", required_argument, nullptr, 'd'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "p:d:h", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'p':
                try {
                    config.port = stoi(optarg);
                    if (config.port < 1 || config.port > 65535) {
                        cerr << "Error: Port must be between 1 and 65535" << endl;
                        exit(EXIT_FAILURE);
                    }
                } catch (...) {
                    cerr << "Error: Invalid port number" << endl;
                    exit(EXIT_FAILURE);
                }
                break;
            case 'd':
                config.db_path = optarg;
                break;
            case 'h':
                print_usage(argv[0]);
                exit(EXIT_SUCCESS);
            default:
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    
    return config;
}

int main(int argc, char* argv[]) {
    try {
        Config config = parse_command_line(argc, argv);
        
        fmt::print("Starting Contract Approval Service...\n");
        fmt::print("Port: {}\n", config.port);
        fmt::print("Database: {}\n", config.db_path);
        
        // Initialize storage
        auto storage = make_unique<storage::SQLiteStorage>(config.db_path);
        storage->init();
        
        // Initialize service layer
        auto contract_service = make_unique<service::ContractService>(move(storage));
        
        // Initialize HTTP handler
        auto handler = make_unique<http::ContractHandler>(move(contract_service));
        
        // Start HTTP server
        httplib::Server server;
        handler->init_routes(server);
        
        fmt::print("Service started successfully on http://localhost:{}\n", config.port);
        fmt::print("Press Ctrl+C to stop...\n");
        
        server.listen("0.0.0.0", config.port);
        
    } catch (const exception& e) {
        cerr << "Service failed to start: " << e.what() << endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
