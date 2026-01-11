#ifndef DATA
#define DATA

#include <string>
#include <vector>

#define SALT "_salt.bin"
#define PUB "_pub.bin"

class Data {
   private:
    Data() {}
    ~Data() {}
    Data(const Data&) = delete;
    Data& operator=(const Data&) = delete;
    Data(Data&&) = delete;
    Data& operator=(Data&&) = delete;

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

    void write_public_key(std::string user, std::vector<char> pub);
    void write_salt(std::string user, std::vector<char> salt);
    std::vector<char> read_public_key(std::string user);
    std::vector<char> read_salt(std::string user);
};

#endif  // DATA