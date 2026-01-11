#ifndef CLIENT
#define CLIENT

#include "comms.hxx"

class Client {
   private:
    Client(const Client& O) = delete;
    Client(Client&& O) = delete;
    Client& operator=(const Client& O) = delete;
    Client& operator=(Client&& O) = delete;

    static void print(std::string mssg);
    static void print_line(std::string mssg);
    static void flush_screen();

    void _run_menu();
    void _register();
    void _authenticate();

    void _actions_menu();

    static int m_running;
    Comms&     m_conn;

   public:
    Client(Comms& conn);
    ~Client() {};

    void start_cli(const char* host, int port);
};

#endif  // CLIENT