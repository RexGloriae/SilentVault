#ifndef DATA
#define DATA

#include <map>
#include <mutex>
#include <string>
#include <vector>

#define SALT "_salt.bin"
#define PUB_KEY "_pub.bin"
#define FILES "_files.dat"
#define INDEXED_FILE(i) ("_file_" + std::to_string(i) + ".bin")

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
    void _add_file_to_index(std::string user, std::vector<char> filename);
    int  _get_index(std::string user, std::vector<char> filename);
    void _remove_index(std::string user, std::vector<char> filename);
    void _update_index(std::string user, int index);

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

    void              write_pub_key(std::string user, std::vector<char> pub);
    std::vector<char> read_pub_key(std::string user);

    void              add_file(std::string       user,
                               std::vector<char> filename,
                               std::vector<char> data);
    std::vector<char> get_file(std::string       user,
                               std::vector<char> filename);
    void delete_file(std::string user, std::vector<char> filename);
    std::vector<std::vector<char>> list_files(std::string user);
};

#endif  // DATA