#ifndef SERVER
#define SERVER

#include <atomic>

#include "comms.hxx"

class Server {
   private:
    static int m_running;

    Comms& m_conn;

    Server(const Server& O) = delete;
    Server(Server&& O) = delete;
    Server operator=(const Server& O) = delete;
    Server operator=(Server&& O) = delete;

    void run_cli();

    static void print(const std::string& text, bool new_line);

   public:
    Server(Comms& conn);
    ~Server();

    static std::atomic<bool> m_should_stop;

    void start();
};

#endif  // SERVER