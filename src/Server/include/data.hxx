#ifndef DATA
#define DATA

#include <map>
#include <mutex>
#include <string>
#include <vector>

#define SALT "_salt.bin"

class Data {
   private:
    Data() {}
    ~Data() {}
    Data(const Data&) = delete;
    Data& operator=(const Data&) = delete;
    Data(Data&&) = delete;
    Data& operator=(Data&&) = delete;

    std::mutex        _mutex;
    std::vector<char> _server_key;
    std::vector<char> _server_iv;

    void              _write_byte_stream(const std::vector<char>& data,
                                         std::string              path);
    std::vector<char> _read_byte_stream(std::string path);

   public:
    static Data& getInstance() {
        static Data instance;
        return instance;
    }

    std::string path(std::string user, std::string type);
    void        create_file(std::string path);
    bool        exists(std::string path);
    void        set_up_secret(std::vector<char> key, std::vector<char> iv);
    bool        check_secret(std::vector<char> key, std::vector<char> iv);

    void set_server_key_iv(std::vector<char> key, std::vector<char> iv);
    std::pair<std::vector<char>, std::vector<char>> get_server_key_iv();

    void              write_salt(std::string user, std::vector<char> salt);
    std::vector<char> read_salt(std::string user);
};

#endif  // DATA