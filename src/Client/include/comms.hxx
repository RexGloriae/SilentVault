#ifndef COMMS
#define COMMS

#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "payload.hxx"

#define SERVER_CERT "../../Server/crypto/server.crt\0"
#define PORT 4433
#define HOSTNAME "127.0.0.1"
#define HOSTNAME_LEN 9
constexpr int MAX_RETRIES = 100;
constexpr int RETRY_DELAY_MS = 500;

class Comms {
   public:
    static Comms& get(void) {
        static Comms instance;
        return instance;
    }

    void              init(const char* host, int port);
    void              send(std::vector<char> payload);
    std::vector<char> recv();

   private:
    Comms() {
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        signal(SIGPIPE, SIG_IGN);
    };
    ~Comms() {
        if (m_ssl) {
            ERR_clear_error();
            SSL_shutdown(m_ssl);
            SSL_free(m_ssl);
        }
        if (m_ctx) {
            SSL_CTX_free(m_ctx);
        }
        if (m_sock >= 0) {
            close(m_sock);
        }
    };
    Comms(const Comms& O) = delete;
    Comms(Comms&& O) = delete;
    Comms& operator=(const Comms& O) = delete;
    Comms& operator=(Comms&& O) = delete;

    SSL_CTX* m_ctx;
    SSL*     m_ssl;
    int      m_sock;
    void     create_context();
    void     tcp_conn(const char* host, int port);
    bool     verify_certificate(const char* host);
};

#endif  // COMMS