#ifndef SERVER
#define SERVER

#include <atomic>
#include <mutex>

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

    void _set_up_encryption_key();
    bool _check_encryption_key();

   public:
    Server(Comms& conn);
    ~Server();

    static std::atomic<bool> m_should_stop;
    static int               _active_clients;
    static std::mutex        console_mutex;
    static void              print(const std::string& text, bool new_line);

    void start();
};

#endif  // SERVER