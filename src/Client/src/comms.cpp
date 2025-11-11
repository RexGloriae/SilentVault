#include "../include/comms.hxx"

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
    m_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sock < 0) {
        throw std::runtime_error("Socket error");
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &addr.sin_addr);

    if (connect(m_sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        throw std::runtime_error("Connection error");
    }

    m_ssl = SSL_new(m_ctx);
    SSL_set_fd(m_ssl, m_sock);

    SSL_set_tlsext_host_name(m_ssl, host);

    if (SSL_connect(m_ssl) != 1) {
        throw std::runtime_error("SSL connect");
    }

    if (verify_certificate(host) == false) {
        throw std::runtime_error("Certificate verification failed");
    }
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
    SSL_write(m_ssl, payload.data(), payload.size());
}

std::vector<char> Comms::recv() {
    std::vector<char> payload(4096);

    int bytes = SSL_read(m_ssl, payload.data(), payload.capacity());
    if (bytes > 0) {
        payload[bytes] = '\0';
        payload.resize(bytes + 1);
    } else
        payload.clear();
    return payload;
}