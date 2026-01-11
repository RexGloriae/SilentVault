#include "../include/data.hxx"

#include <fstream>

std::string Data::path(std::string user, std::string type) {
    return "../data/" + user + type;
}
void Data::create_file(std::string path) {
    std::ofstream file(path, std::ios::app);
    file.close();
}
bool Data::exists(std::string path) {
    std::ifstream file(path);
    bool          answ = file.good();
    file.close();
    return answ;
}

void Data::write_public_key(std::string user, std::vector<char> pub) {
    _write_byte_stream(pub, path(user, PUB));
}
void Data::write_salt(std::string user, std::vector<char> salt) {
    _write_byte_stream(salt, path(user, SALT));
}
std::vector<char> Data::read_public_key(std::string user) {
    return _read_byte_stream(path(user, PUB));
}
std::vector<char> Data::read_salt(std::string user) {
    return _read_byte_stream(path(user, SALT));
}

void Data::_write_byte_stream(const std::vector<char>& data,
                              std::string              path) {
    std::ofstream fout(path, std::ios::binary);
    size_t        sz = data.size();
    fout.write(reinterpret_cast<const char*>(&sz), sizeof(size_t));
    fout.write(data.data(), data.size());
    fout.close();
}
std::vector<char> Data::_read_byte_stream(std::string path) {
    std::ifstream fin(path, std::ios::binary);
    size_t        size;
    fin.read(reinterpret_cast<char*>(&size), sizeof(size_t));
    char* buff = new char[size];
    fin.read(buff, size);
    fin.close();
    std::vector<char> data(buff, buff + size);
    delete[] buff;
    return data;
}