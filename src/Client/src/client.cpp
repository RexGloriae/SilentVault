#include "../include/client.hxx"

#include <stdexcept>

int Client::m_running = 0;

Client::Client(Comms& conn) : m_conn(conn) {
    m_running++;
    if (m_running > 1) {
        throw std::runtime_error("You can't declare more than one client");
    }
}

void Client::start(const char* host, int port) { m_conn.init(host, port); }