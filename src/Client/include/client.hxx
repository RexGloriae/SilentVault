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
    void _generate_secret(std::string pass);

    void _actions_menu();

    void _upload();
    void _download();
    void _see_list();
    void _delete();

    std::vector<char> _key;
    std::vector<char> _iv;
    std::string       _user;

    static int m_running;
    Comms&     m_conn;

   public:
    Client(Comms& conn);
    ~Client() {};

    void start_cli(const char* host, int port);

    std::vector<char> key() { return _key; }
    std::vector<char> iv() { return _iv; }
};

#endif  // CLIENT