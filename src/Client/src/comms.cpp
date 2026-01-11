#include "../include/comms.hxx"

#include <chrono>
#include <thread>

#include "../include/client.hxx"

void Comms::init(const char* host, int port) {
    create_context();
    tcp_conn(host, port);
}

void Comms::create_context() {
    const SSL_METHOD* method = TLS_client_method();
    m_ctx = SSL_CTX_new(method);
    if (!m_ctx) {
        throw std::runtime_error("Unable to create SSL context\n");
    }

    if (SSL_CTX_load_verify_locations(m_ctx, SERVER_CERT, nullptr) != 1) {
        throw std::runtime_error("Certificate not trusted");
    }

    SSL_CTX_set_verify(m_ctx, SSL_VERIFY_PEER, nullptr);
    SSL_CTX_set_verify_depth(m_ctx, 4);
}

void Comms::tcp_conn(const char* host, int port) {
    for (int attempt = 1; attempt <= MAX_RETRIES; ++attempt) {
        m_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (m_sock < 0) {
            throw std::runtime_error("Socket error");
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);

        if (inet_pton(AF_INET, host, &addr.sin_addr) != 1) {
            close(m_sock);
            throw std::runtime_error("Invalid address");
        }

        if (connect(m_sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
            if (errno == ECONNREFUSED || errno == ETIMEDOUT ||
                errno == EHOSTUNREACH) {
                close(m_sock);
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(RETRY_DELAY_MS));
                continue;  // retry
            }

            close(m_sock);
            throw std::runtime_error("Connection error");
        }

        // TCP connected — now TLS
        m_ssl = SSL_new(m_ctx);
        if (!m_ssl) {
            close(m_sock);
            throw std::runtime_error("SSL_new failed");
        }

        SSL_set_fd(m_ssl, m_sock);
        SSL_set_tlsext_host_name(m_ssl, host);

        if (SSL_connect(m_ssl) != 1) {
            SSL_free(m_ssl);
            close(m_sock);

            std::this_thread::sleep_for(
                std::chrono::milliseconds(RETRY_DELAY_MS));
            continue;  // retry TLS
        }

        if (!verify_certificate(host)) {
            SSL_free(m_ssl);
            close(m_sock);
            throw std::runtime_error("Certificate verification failed");
        }

        std::cout << "\n[Comms] >> SSL connection established (attempt "
                  << attempt << ")...\n";

        return;  // SUCCESS
    }

    throw std::runtime_error("Server not available after retries");
}

bool Comms::verify_certificate(const char* host) {
    X509* cert = SSL_get_peer_certificate(m_ssl);
    if (!cert) return false;

    long result = SSL_get_verify_result(m_ssl);
    if (result != X509_V_OK) return false;

    if (X509_check_host(cert, host, strlen(host), 0, nullptr) != 1)
        return false;

    X509_free(cert);
    return true;
}

void Comms::send(std::vector<char> payload) {
    int len = payload.size();
    SSL_write(m_ssl, reinterpret_cast<char*>(&len), sizeof(int));
    SSL_write(m_ssl, payload.data(), payload.size());
}

std::vector<char> Comms::recv() {
    int len = 0;
    int bytes_read = 0;
    int total_read = 0;

    // 1. Read the length (4 bytes)
    char* len_buf = reinterpret_cast<char*>(&len);
    while (total_read < static_cast<int>(sizeof(int))) {
        bytes_read = SSL_read(m_ssl, len_buf + total_read,
                              sizeof(int) - total_read);
        if (bytes_read <= 0) {
            // Check for actual error vs graceful shutdown if needed,
            // but for a robust client, 0 or <0 here usually means we can't proceed.
            // SSL_get_error might provide more info, but throwing is safest.
            throw std::runtime_error("Connection lost or error reading length");
        }
        total_read += bytes_read;
    }

    // Safety check: Sanitizing input length to prevent huge allocations
    // Let's assume 10MB limit for now, adjust as needed.
    if (len < 0 || len > 10 * 1024 * 1024) {
        throw std::runtime_error("Invalid payload length received");
    }

    if (len == 0) {
        return std::vector<char>();
    }

    // 2. Read the body
    std::vector<char> payload(len);
    total_read = 0;
    while (total_read < len) {
        bytes_read = SSL_read(m_ssl, payload.data() + total_read,
                              len - total_read);
        if (bytes_read <= 0) {
             throw std::runtime_error("Connection lost or error reading body");
        }
        total_read += bytes_read;
    }

    return payload;
}