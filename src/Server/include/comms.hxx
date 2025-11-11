#ifndef COMMS
#define COMMS

#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <condition_variable>
#include <cstring>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#define PORT "4433"
#define CERT_FILE "../crypto/server.crt"
#define KEY_FILE "../crypto/server.key"

class Comms {
   public:
    static Comms& get(void) {
        static Comms instance;
        return instance;
    }

    void start_and_listen(void);

   private:
    Comms() {
        SSL_load_error_strings();
        OpenSSL_add_ssl_algorithms();
    };
    ~Comms() {
        SSL_CTX_free(m_ctx);
        EVP_cleanup();
    };
    Comms(const Comms& O) = delete;
    Comms(Comms&& O) = delete;
    Comms& operator=(const Comms& O) = delete;
    Comms& operator=(Comms&& O) = delete;

    SSL_CTX* m_ctx;
    int      m_sock;

    void create_context(const char* cert_file, const char* key_file);
    void create_listen_sock(const char* port);
    void handle_client(int sock);
};

#endif  // COMMS