#include "../include/server.hxx"

#include <cstring>
#include <stdexcept>
#include <thread>

int               Server::m_running = 0;
std::atomic<bool> Server::m_should_stop = false;

Server::Server(Comms& conn) : m_conn(conn) {
    m_running++;
    if (m_running > 1) {
        throw std::runtime_error("You can't declare more than one server");
    }
}

Server::~Server() {};

void Server::run_cli() {
    Server::print("Enter \"help\" to see available commands.", true);
    std::string cmd;
    bool        run = true;
    while (run) {
        std::cin >> cmd;
        if (strcasecmp("stop", cmd.c_str()) == 0) {
            run = false;
            m_should_stop = true;
            m_conn.close_sock();
        } else if (strcasecmp("stats", cmd.c_str()) == 0) {
        }
    }
}

void Server::print(const std::string& text, bool new_line) {
    if (new_line == true) std::cout << "\n";
    std::cout << "Server >$ " << text << std::endl;
    std::cout << "\nServer >$ ";
}

void Server::start() {
    std::thread cli_thread(&Server::run_cli, this);
    m_conn.start_and_listen();
    cli_thread.join();
}
