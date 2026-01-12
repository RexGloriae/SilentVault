#include "../include/data.hxx"

#include <filesystem>
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
void Data::write_pub_key(std::string user, std::vector<char> pub) {
    PythonWrapper&    wrapper = PythonWrapper::get();
    std::vector<char> cipher =
        wrapper.encrypt(_server_key, _server_iv, pub);

    _write_byte_stream(cipher, path(user, PUB_KEY));
}
std::vector<char> Data::read_pub_key(std::string user) {
    PythonWrapper&    wrapper = PythonWrapper::get();
    std::vector<char> cipher = _read_byte_stream(path(user, PUB_KEY));
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
void Data::_add_file_to_index(std::string       user,
                              std::vector<char> filename) {
    std::string   index_path = path(user, FILES);
    std::ifstream fin(index_path, std::ios::binary);
    int           total_files;
    if (!fin.is_open()) {
        total_files = 0;
        create_file(index_path);
    } else {
        fin.read(reinterpret_cast<char*>(&total_files), sizeof(int));
    }
    total_files++;
    std::vector<std::vector<char>> files;
    for (int i = 0; i < total_files - 1; i++) {
        int               fname_len;
        std::vector<char> fname;
        fin.read(reinterpret_cast<char*>(&fname_len), sizeof(int));
        fname.resize(fname_len);
        fin.read(fname.data(), fname_len);
        files.push_back(fname);
    }
    files.push_back(filename);
    fin.close();
    std::ofstream fout(index_path, std::ios::binary);
    fout.write(reinterpret_cast<const char*>(&total_files), sizeof(int));
    for (int i = 0; i < total_files; i++) {
        int fname_len = files[i].size();
        fout.write(reinterpret_cast<const char*>(&fname_len), sizeof(int));
        fout.write(files[i].data(), fname_len);
    }
}
int Data::_get_index(std::string user, std::vector<char> filename) {
    std::string   index_path = path(user, FILES);
    std::ifstream fin(index_path, std::ios::binary);
    int           total_files;
    fin.read(reinterpret_cast<char*>(&total_files), sizeof(int));
    for (int i = 0; i < total_files; i++) {
        int               fname_len;
        std::vector<char> fname;
        fin.read(reinterpret_cast<char*>(&fname_len), sizeof(int));
        fname.resize(fname_len);
        fin.read(fname.data(), fname_len);
        if (fname == filename) {
            fin.close();
            return i;
        }
    }
    fin.close();
    return -1;  // not found
}
void Data::_remove_index(std::string user, std::vector<char> filename) {
    std::string   index_path = path(user, FILES);
    std::ifstream fin(index_path, std::ios::binary);
    int           total_files;
    fin.read(reinterpret_cast<char*>(&total_files), sizeof(int));
    std::vector<std::vector<char>> all_files;
    bool                           update_index = false;
    for (int i = 0; i < total_files; i++) {
        if (update_index == true) {
            int           new_index = i - 1;
            std::ifstream tmp(path(user, INDEXED_FILE(i)),
                              std::ios::binary);
            tmp.seekg(0, std::ios::end);
            size_t file_size = tmp.tellg();
            tmp.seekg(0, std::ios::beg);
            std::vector<char> file_data(file_size);
            tmp.read(file_data.data(), file_size);
            tmp.close();
            std::ofstream tmp_out(path(user, INDEXED_FILE(new_index)),
                                  std::ios::binary);
            tmp_out.write(file_data.data(), file_size);
            tmp_out.close();
            std::filesystem::remove(path(user, INDEXED_FILE(i)));
        }
        int               fname_len;
        std::vector<char> fname;
        fin.read(reinterpret_cast<char*>(&fname_len), sizeof(int));
        fname.resize(fname_len);
        fin.read(fname.data(), fname_len);
        if (fname == filename) {
            update_index = true;
            continue;
        }
        all_files.push_back(fname);
    }
    fin.close();
    std::ofstream fout(index_path, std::ios::binary);
    int           new_total = total_files - 1;
    fout.write(reinterpret_cast<const char*>(&new_total), sizeof(int));
    for (size_t i = 0; i < all_files.size(); i++) {
        int fname_len = all_files[i].size();
        fout.write(reinterpret_cast<const char*>(&fname_len), sizeof(int));
        fout.write(all_files[i].data(), fname_len);
    }
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

void Data::add_file(std::string       user,
                    std::vector<char> filename,
                    std::vector<char> data) {
    std::string       file_path;
    PythonWrapper&    wrapper = PythonWrapper::get();
    std::vector<char> cipher_filename =
        wrapper.encrypt(_server_key, _server_iv, filename);
    _add_file_to_index(user, cipher_filename);
    file_path =
        path(user, INDEXED_FILE(_get_index(user, cipher_filename)));

    _write_byte_stream(wrapper.encrypt(_server_key, _server_iv, data),
                       file_path);
}
std::vector<char> Data::get_file(std::string       user,
                                 std::vector<char> filename) {
    PythonWrapper&    wrapper = PythonWrapper::get();
    std::vector<char> cipher_filename =
        wrapper.encrypt(_server_key, _server_iv, filename);
    int               index = _get_index(user, cipher_filename);
    std::string       file_path = path(user, INDEXED_FILE(index));
    std::vector<char> file_data = _read_byte_stream(file_path);
    return wrapper.decrypt(_server_key, _server_iv, file_data);
}
void Data::delete_file(std::string user, std::vector<char> filename) {
    PythonWrapper&    wrapper = PythonWrapper::get();
    std::vector<char> cipher_filename =
        wrapper.encrypt(_server_key, _server_iv, filename);
    _remove_index(user, cipher_filename);
}
std::vector<std::vector<char>> Data::list_files(std::string user) {
    PythonWrapper& wrapper = PythonWrapper::get();
    std::string    index_path = path(user, FILES);
    std::ifstream  fin(index_path, std::ios::binary);
    if (!fin.is_open()) {
        return {};
    }
    int total_files;
    fin.read(reinterpret_cast<char*>(&total_files), sizeof(int));
    std::vector<std::vector<char>> files;
    for (int i = 0; i < total_files; i++) {
        int               fname_len;
        std::vector<char> fname;
        fin.read(reinterpret_cast<char*>(&fname_len), sizeof(int));
        fname.resize(fname_len);
        fin.read(fname.data(), fname_len);
        // files.push_back(fname);
        files.push_back(wrapper.decrypt(_server_key, _server_iv, fname));
    }
    fin.close();
    return files;
}