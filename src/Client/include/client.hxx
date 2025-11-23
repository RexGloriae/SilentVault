#ifndef CLIENT
#define CLIENT

#include "comms.hxx"

class Client {
   private:
    Client(const Client& O) = delete;
    Client(Client&& O) = delete;
    Client& operator=(const Client& O) = delete;
    Client& operator=(Client&& O) = delete;

    static int m_running;
    Comms&     m_conn;

   public:
    Client(Comms& conn);
    ~Client();

    void start(const char* host, int port);
};

#endif  // CLIENT