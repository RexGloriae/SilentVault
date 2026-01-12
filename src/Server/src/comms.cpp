#include "../include/comms.hxx"

#include <stdexcept>

#include "../include/payload.hxx"
#include "../include/resolver.hxx"
#include "../include/server.hxx"

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
    while (Server::m_should_stop == false) {
        struct sockaddr_in addr;
        socklen_t          len = sizeof(addr);
        int client_sock = accept(m_sock, (struct sockaddr*)&addr, &len);
        if (client_sock < 0) {
            // log error strerror(errno);
            if (Server::m_should_stop == false) break;
            continue;
        }

        if (client_sock < 0) {
            if (Server::m_should_stop) break;
            continue;
        }
        std::string client_id = std::string(inet_ntoa(addr.sin_addr)) +
                                ":" + std::to_string(ntohs(addr.sin_port));
        Server::print("Client connected: " + client_id + "...", true);
        std::thread t(std::bind(
            &Comms::handle_client, this, client_sock, client_id));
        t.detach();
    }
}

void Comms::handle_client(int sock, std::string client_id) {
    SSL* ssl = nullptr;
    try {
        ssl = SSL_new(m_ctx);
        SSL_set_fd(ssl, sock);

        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
            throw std::runtime_error("error accepting client");
        }

        // TODO: implement client handling
        // Loop to handle requests from this client
        bool run = true;
        while (run) {
            int len = 0;
            int bytes_read = 0;
            int total_read = 0;

            // 1. Read the length (4 bytes)
            char* len_buf = reinterpret_cast<char*>(&len);
            while (total_read < static_cast<int>(sizeof(int))) {
                bytes_read = SSL_read(
                    ssl, len_buf + total_read, sizeof(int) - total_read);
                if (bytes_read <= 0) {
                    // Client disconnected or error
                    run = false;
                    break;
                }
                total_read += bytes_read;
            }
            if (!run) break;

            // Safety check
            if (len < 0 || len > 10 * 1024 * 1024) {
                std::cerr << "Invalid length received: " << len
                          << std::endl;
                run = false;
                break;
            }
            if (len == 0) {
                // Empty payload? Just continue (if valid) or close?
                // Assuming continue for now, but watch for infinite loops
                // if 0-byte reads persist (should be covered by bytes_read
                // check)
                continue;
            }

            // 2. Read the body
            std::vector<char> buff(len);
            total_read = 0;
            while (total_read < len) {
                bytes_read = SSL_read(
                    ssl, buff.data() + total_read, len - total_read);
                if (bytes_read <= 0) {
                    run = false;
                    break;
                }
                total_read += bytes_read;
            }
            if (!run) break;

            std::vector<char> payload = buff;
            Resolver&         solver = Resolver::get();
            IPayload* request = PayloadCreator::interpret_payload(payload);
            if (request == nullptr) {
                // Unknown opcode or empty
                continue;
            }

            if (request->deserialize() == false) {
                delete request;
                // log bad request
                continue;
            }
            IPayload* response = solver.solve(request, client_id);
            if (response == nullptr) {
                delete request;
                continue;
            }
            std::vector<char> reply = response->serialize();
            len = reply.size();
            SSL_write(ssl, reinterpret_cast<char*>(&len), sizeof(int));
            int ret = SSL_write(
                ssl, reply.data(), static_cast<int>(reply.size()));
            if (ret <= 0) {
                // Handle write error
                ERR_print_errors_fp(stderr);
            }
            delete request;
            delete response;
        }

        ERR_clear_error();
        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(sock);
    } catch (const std::exception& e) {
        // Log the exception if needed
        if (ssl) {
            ERR_clear_error();
            SSL_shutdown(ssl);
            SSL_free(ssl);
        }
        close(sock);
    }
    Server::print("Client " + client_id + " has disconnected...", true);
}

void Comms::close_sock() {
    if (m_sock >= 0) {
        shutdown(m_sock, SHUT_RDWR);
        close(m_sock);
        m_sock = -1;
    }
}