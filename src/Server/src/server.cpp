#include "../include/server.hxx"

#include <cstring>
#include <stdexcept>
#include <thread>

#include "../../PythonWrapper/wrap.hxx"
#include "../include/data.hxx"

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
    Data& data = Data::getInstance();
    if (data.exists("../data/setup_key.bin") == false) {
        _set_up_encryption_key();
    } else if (_check_encryption_key() == false) {
        throw std::runtime_error("Wrong encryption key provided!");
    }

    std::thread cli_thread(&Server::run_cli, this);
    m_conn.start_and_listen();
    cli_thread.join();
}

void Server::_set_up_encryption_key() {
    std::string pass;
    Server::print("Set encryption key for server data: ", false);
    std::cin >> pass;

    PythonWrapper& wrapper = PythonWrapper::get();
    std::string    key_hash = wrapper.sha256(pass);

    std::vector<char> key_vec;
    std::vector<char> iv_vec;
    int               middle = key_hash.size() / 2;
    int               size = key_hash.size();
    for (int i = 0; i < middle; i++) {
        key_vec.push_back(key_hash[i] ^ key_hash[size - 1 - i]);
    }
    middle = key_vec.size() / 2;
    size = key_vec.size();
    for (int i = 0; i < middle; i++) {
        iv_vec.push_back(key_vec[i] ^ key_vec[size - 1 - i]);
    }

    Data& data = Data::getInstance();
    data.set_up_secret(key_vec, iv_vec);
    data.set_server_key_iv(key_vec, iv_vec);
}

bool Server::_check_encryption_key() {
    std::string pass;
    Server::print("Enter encryption key for server data: ", false);
    std::cin >> pass;

    PythonWrapper& wrapper = PythonWrapper::get();
    std::string    key_hash = wrapper.sha256(pass);

    std::vector<char> key_vec;
    std::vector<char> iv_vec;
    int               middle = key_hash.size() / 2;
    int               size = key_hash.size();
    for (int i = 0; i < middle; i++) {
        key_vec.push_back(key_hash[i] ^ key_hash[size - 1 - i]);
    }
    middle = key_vec.size() / 2;
    size = key_vec.size();
    for (int i = 0; i < middle; i++) {
        iv_vec.push_back(key_vec[i] ^ key_vec[size - 1 - i]);
    }

    Data& data = Data::getInstance();
    data.set_server_key_iv(key_vec, iv_vec);
    return data.check_secret(key_vec, iv_vec);
}