#include "../include/data.hxx"

#include <fstream>
#include <mutex>

#include "../../PythonWrapper/wrap.hxx"

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

void Data::write_salt(std::string user, std::vector<char> salt) {
    PythonWrapper&    wrapper = PythonWrapper::get();
    std::vector<char> cipher =
        wrapper.encrypt(_server_key, _server_iv, salt);

    _write_byte_stream(cipher, path(user, SALT));
}
std::vector<char> Data::read_salt(std::string user) {
    PythonWrapper&    wrapper = PythonWrapper::get();
    std::vector<char> cipher = _read_byte_stream(path(user, SALT));
    return wrapper.decrypt(_server_key, _server_iv, cipher);
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

void Data::set_up_secret(std::vector<char> key, std::vector<char> iv) {
    PythonWrapper&    wrapper = PythonWrapper::get();
    std::vector<char> encrypted = wrapper.encrypt(
        key,
        iv,
        std::vector<char>{
            's', 'e', 'c', 'r', 'e', 't', '_', 'k', 'e', 'y'});
    _write_byte_stream(encrypted, "../data/setup_key.bin");
}
bool Data::check_secret(std::vector<char> key, std::vector<char> iv) {
    std::vector<char> encrypted =
        _read_byte_stream("../data/setup_key.bin");
    PythonWrapper&    wrapper = PythonWrapper::get();
    std::vector<char> decrypted = wrapper.decrypt(key, iv, encrypted);
    std::vector<char> correct{
        's', 'e', 'c', 'r', 'e', 't', '_', 'k', 'e', 'y'};
    return decrypted == correct;
}

void Data::set_server_key_iv(std::vector<char> key, std::vector<char> iv) {
    std::lock_guard<std::mutex> lock(_mutex);
    _server_key = key;
    _server_iv = iv;
}

std::pair<std::vector<char>, std::vector<char>> Data::get_server_key_iv() {
    std::lock_guard<std::mutex> lock(_mutex);
    return std::make_pair(_server_key, _server_iv);
}