#include "../include/comms.hxx"

#include <stdexcept>

void Comms::create_context(const char* cert_file, const char* key_file) {
    const SSL_METHOD* method = TLS_server_method();
    m_ctx = SSL_CTX_new(method);
    if (!m_ctx) {
        throw std::runtime_error("Unable to create SSL Context");
    }
    SSL_CTX_set_min_proto_version(m_ctx, TLS1_2_VERSION);

    if (SSL_CTX_use_certificate_file(m_ctx, cert_file, SSL_FILETYPE_PEM) <=
        0) {
        unsigned long err = ERR_get_error();
        std::cerr << "OpenSSL error: " << ERR_error_string(err, nullptr)
                  << std::endl;
        throw std::runtime_error("Unable to load certificate");
    }

    if (SSL_CTX_use_PrivateKey_file(m_ctx, key_file, SSL_FILETYPE_PEM) <=
        0) {
        throw std::runtime_error("Unable to load private key");
    }

    if (!SSL_CTX_check_private_key(m_ctx)) {
        throw std::runtime_error(
            "Private key does not match certificate public key");
    }
}

void Comms::create_listen_sock(const char* port) {
    struct addrinfo hints{}, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int rc = getaddrinfo(nullptr, port, &hints, &res);
    if (rc != 0) {
        throw std::runtime_error("getaddrinfo: " +
                                 std::string(gai_strerror(rc)));
    }

    m_sock = -1;
    for (struct addrinfo* p = res; p != nullptr; p = p->ai_next) {
        m_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (m_sock < 0) continue;

        int opt = 1;
        setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        if (bind(m_sock, p->ai_addr, p->ai_addrlen) == 0) {
            if (listen(m_sock, SOMAXCONN) == 0) {
                break;
            }
        }
        close(m_sock);
        m_sock = -1;
    }
    freeaddrinfo(res);
}

void Comms::start_and_listen() {
    create_context(CERT_FILE, KEY_FILE);
    create_listen_sock(PORT);
    while (true) {
        struct sockaddr_in addr;
        socklen_t          len = sizeof(addr);
        int client_sock = accept(m_sock, (struct sockaddr*)&addr, &len);
        if (client_sock < 0) {
            // log error strerror(errno);
            continue;
        }
        std::thread t(std::bind(&Comms::handle_client, this, client_sock));
        t.detach();
    }
}

void Comms::handle_client(int sock) {
    SSL* ssl = SSL_new(m_ctx);
    SSL_set_fd(ssl, sock);

    if (SSL_accept(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(sock);
        throw std::runtime_error("error accepting client");
    }

    // TODO: implement client handling
    char buff[4096];
    int  bytes = SSL_read(ssl, buff, sizeof(buff) - 1);
    if (bytes > 0) {
        std::cout << std::string(buff, bytes) << std::endl;
        std::string reply = "Server got your message";
        SSL_write(ssl, reply.c_str(), (int)reply.size());
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sock);
}