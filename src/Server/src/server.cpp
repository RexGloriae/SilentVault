#include "../include/server.hxx"

#include <cstring>
#include <stdexcept>
#include <thread>

#include "../../PythonWrapper/wrap.hxx"

int               Server::m_running = 0;
std::atomic<bool> Server::m_should_stop = false;
std::mutex        Server::console_mutex;

Server::Server(Comms& conn) : m_conn(conn) {
    m_running++;
    if (m_running > 1) {
        throw std::runtime_error("You can't declare more than one server");
    }
}

Server::~Server() {};

void Server::run_cli() {
    std::string line;
    while (!m_should_stop) {
        std::cout << "Server >$ " << std::flush;

        if (!std::getline(std::cin, line)) {
            break;
        }

        if (line.empty()) continue;

        if (strcasecmp("stop", line.c_str()) == 0) {
            Server::print("Shutting down...", true);
            m_should_stop = true;
            m_conn.close_sock();
            break;
        } else if (strcasecmp("stats", line.c_str()) == 0) {
            Server::print("Active Clients: [Feature not implemented]",
                          true);
        } else if (strcasecmp("help", line.c_str()) == 0) {
            Server::print("Available commands: stop, stats, help", true);
        } else {
            Server::print("Unknown command: " + line, true);
        }
    }
}

void Server::print(const std::string& text, bool new_line) {
    std::lock_guard<std::mutex> lock(Server::console_mutex);
    std::cout << "\r" << std::string(80, ' ') << "\r";
    std::cout << "Server >$ " << text;
    if (new_line == true) std::cout << std::endl;
    std::cout << "Server >$ " << std::flush;
}

void Server::start() {
    PythonWrapper::get();
    std::thread cli_thread(&Server::run_cli, this);
    m_conn.start_and_listen();
    cli_thread.join();
}
