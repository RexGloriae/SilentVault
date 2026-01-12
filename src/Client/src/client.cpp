#include "../include/client.hxx"

#include <filesystem>
#include <fstream>
#include <stdexcept>

#include "../../PythonWrapper/wrap.hxx"

int Client::m_running = 0;

void Client::print(std::string mssg) {
    std::cout << "[Client] >> " + mssg;
}
void Client::print_line(std::string mssg) {
    std ::cout << "[Client] >> " + mssg << std::endl;
}
void Client::flush_screen() {
    std::cout << std::endl;
    for (int i = 0; i < 50; i++) {
        std::cout << "-";
    }
    std::cout << std::endl;
}

Client::Client(Comms& conn) : m_conn(conn) {
    m_running++;
    if (m_running > 1) {
        throw std::runtime_error("You can't declare more than one client");
    }
}

void Client::start_cli(const char* host, int port) {
    Client::print_line("Establishing connection to the server...");
    try {
        m_conn.init(host, port);
    } catch (const std::runtime_error& E) {
        std::cout << E.what() << std::endl;
        return;
    }

    Client::print_line("Connection to the server established...");
    _run_menu();
}

void Client::_run_menu() {
    int  option;
    bool _try = true;
    while (_try) {
        Client::flush_screen();
        Client::print_line("Choose an option: ");
        std::cout << "1. Authenticate\n";
        std::cout << "2. Register\n";
        std::cout << "3. Exit\n";
        std::cout << "Type your option: ";
        std::cin >> option;
        switch (option) {
            case 1:
                Client::flush_screen();
                try {
                    _authenticate();
                    _actions_menu();
                } catch (const std::exception& e) {
                    std::cout
                        << "\nError during authentication: " << e.what()
                        << "\n";
                    std::cout << "Press Enter to continue...";
                    std::cin.ignore();
                    std::cin.get();
                }
                break;
            case 2:
                Client::flush_screen();
                try {
                    _register();
                } catch (const std::exception& e) {
                    std::cout
                        << "\nError during registration: " << e.what()
                        << "\n";
                    std::cout << "Press Enter to continue...";
                    std::cin.ignore();
                    std::cin.get();
                }
                break;
            case 3:
                _try = false;
                break;
            default:
                std::cout << "Not a valid option! Try again...\n";
                break;
        }
    }
}

void Client::_register() {
    PythonWrapper& wrapper = PythonWrapper::get();
    Client::print("Enter a username: ");
    std::string user;
    std::cin >> user;
    Client::print("Enter a password: ");
    std::string pass;
    std::cin >> pass;
    // pub, salt
    Client::print_line("Generating public key and salt from secret...");
    std::pair<std::vector<char>, std::vector<char>> result =
        wrapper.client_pub_from_pass(pass);
    // send public and salt to server
    Client::print_line("Sending salt to server...");
    PostSaltPayload payload(user, result.second);
    m_conn.send(payload.serialize());
}

void Client::_authenticate() {
    PythonWrapper& wrapper = PythonWrapper::get();
    Client::print("Enter username: ");
    std::string user;
    std::cin >> user;
    Client::print("Enter password: ");
    std::string pass;
    std::cin >> pass;

    // request from server pub and salt
    PostSaltRequestPayload req_payload(user);
    Client::print_line("Requesting salt from server...");
    m_conn.send(req_payload.serialize());
    std::vector<char> buff = m_conn.recv();
    GetSaltPayload    payload(buff);
    if (payload.deserialize() == false) {
        Client::print("User not found!");
        throw std::runtime_error("user not found");
    }
    Client::print_line("Generating public key and salt from secret...");
    std::pair<std::vector<char>, std::vector<char>> pub_salt_pair =
        wrapper.client_pub_from_pass(pass, payload.salt());

    // R bytes, r int
    Client::print_line("Creating commit...");
    std::pair<std::vector<char>, std::vector<char>> result =
        wrapper.client_commit(pass, pub_salt_pair.second);

    PostChallengeRequestPayload chall_req_payload(user);
    Client::print_line("Requesting challenge from server...");
    m_conn.send(chall_req_payload.serialize());

    buff.clear();
    buff = m_conn.recv();
    GetChallengePayload challenge_payload(buff);
    if (challenge_payload.deserialize() == false) {
        Client::print("Error receiving challenge from server!");
        throw std::runtime_error("error receiving challenge");
    }
    std::vector<char> challenge = challenge_payload.challenge();
    Client::print_line("Computing response for given challenge...");
    std::vector<char> response = wrapper.client_compute_response(
        pass, result.second, pub_salt_pair.second, challenge);
    // send response
    PostResponsePayload resp_payload(
        pub_salt_pair.first, response, result.first, challenge, user);
    Client::print_line("Sending response to server...");
    m_conn.send(resp_payload.serialize());
    buff.clear();
    buff = m_conn.recv();
    GetAuthResponsePayload auth_resp_payload(buff);
    if (auth_resp_payload.deserialize() == false) {
        Client::print("Error receiving auth response from server!");
        throw std::runtime_error("error receiving auth response");
    }
    bool ok = auth_resp_payload.success();
    // receive ok from server
    if (ok == false) {
        Client::print("Bad Authentication!");
        throw std::runtime_error("bad auth");
    }
    Client::print_line("Authentication OK...");
    _user = user;
    _generate_secret(pass);
}

void Client::_generate_secret(std::string pass) {
    PythonWrapper& wrapper = PythonWrapper::get();
    std::string    key_hash = wrapper.sha256(pass);
    int            middle = key_hash.size() / 2;
    int            size = key_hash.size();
    _key.clear();
    _iv.clear();
    for (int i = 0; i < middle; i++) {
        _key.push_back(key_hash[i] ^ key_hash[size - 1 - i]);
    }
    middle = _key.size() / 2;
    size = _key.size();
    for (int i = 0; i < middle; i++) {
        _iv.push_back(_key[i] ^ _key[size - 1 - i]);
    }
}

void Client::_actions_menu() {
    int  option;
    bool _try = true;
    while (_try) {
        Client::flush_screen();
        Client::print_line("Choose an option: ");
        std::cout << "1. Upload a file\n";
        std::cout << "2. Download a File\n";
        std::cout << "3. See File List\n";
        std::cout << "4. Delete a File\n";
        std::cout << "5. Log out\n";
        std::cout << "Type your option: ";
        std::cin >> option;
        switch (option) {
            case 1:
                Client::flush_screen();
                try {
                    _upload();
                } catch (const std::exception& e) {
                    std::cout << "Error while uploading: " << e.what();
                    std::cin.ignore();
                    std::cin.get();
                }
                break;
            case 2:
                Client::flush_screen();
                try {
                    _download();
                } catch (const std::exception& e) {
                    std::cout << "Error while uploading: " << e.what();
                    std::cin.ignore();
                    std::cin.get();
                }
                break;
            case 3:
                Client::flush_screen();
                try {
                    _see_list();
                } catch (const std::exception& e) {
                    std::cout << "Error while fetching list: " << e.what();
                    std::cin.ignore();
                    std::cin.get();
                }
                break;
            case 4:
                Client::flush_screen();
                try {
                    _delete();
                } catch (const std::exception& e) {
                    std::cout << "Error while deleting file: " << e.what();
                    std::cin.ignore();
                    std::cin.get();
                }
                break;
            case 5:
                _try = false;
                break;
            default:
                std::cout << "Not a valid option! Try again...\n";
                break;
        }
    }
}

void Client::_upload() {
    Client::print("Enter path of file to upload: ");
    std::string path;
    std::cin >> path;
    PythonWrapper& wrapper = PythonWrapper::get();
    Client::print_line("Zipping file...");
    wrapper.zip_files({path}, "../tmp/temp_archive.zip");
    std::ifstream file("../tmp/temp_archive.zip", std::ios::binary);
    if (!file.is_open()) {
        Client::print_line("Error opening file!");
        return;
    }
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> file_data(file_size);
    Client::print_line("Reading archive...");
    file.read(file_data.data(), file_size);
    file.close();
    Client::print_line("Encrypting data...");
    std::vector<char> encrypted_data =
        wrapper.encrypt(_key, _iv, file_data);

    std::string filename = std::filesystem::path(path).filename().string();
    std::vector<char> filename_cipher = wrapper.encrypt(
        _key, _iv, std::vector<char>(filename.begin(), filename.end()));

    Client::print_line("Sending data to server...");
    PostUploadFile payload(encrypted_data, filename_cipher, _user);
    m_conn.send(payload.serialize());
    std::filesystem::remove("../tmp/temp_archive.zip");
}
void Client::_download() {
    Client::print("Enter filename to download: ");
    std::string filename;
    std::cin >> filename;
    std::vector<char> filename_cipher = PythonWrapper::get().encrypt(
        _key, _iv, std::vector<char>(filename.begin(), filename.end()));
    // send to server request to download
    PostDownloadRequest payload(filename_cipher, _user);
    Client::print_line("Requesting file from server...");
    m_conn.send(payload.serialize());
    GetDownloadFile resp_payload(m_conn.recv());
    if (resp_payload.deserialize() == false) {
        Client::print_line("Error receiving file from server!");
        return;
    }
    Client::print_line("Decrypting data...");
    std::vector<char> plain_data =
        PythonWrapper::get().decrypt(_key, _iv, resp_payload.file_data());
    Client::print("Where to save the downloaded file: ");
    std::string save_path;
    std::cin >> save_path;
    std::ofstream outfile("../tmp/tmp_archive.zip", std::ios::binary);
    Client::print_line("Writing data...");
    outfile.write(reinterpret_cast<const char*>(plain_data.data()),
                  plain_data.size());
    outfile.close();
    Client::print_line("Unzipping file...");
    PythonWrapper::get().unzip("../tmp/tmp_archive.zip", save_path);
    std::filesystem::remove("../tmp/tmp_archive.zip");
}
void Client::_see_list() {
    std::vector<std::string> files;
    PostListRequest          payload(_user);
    Client::print_line("Requesting file list from server...");
    m_conn.send(payload.serialize());
    GetListResponse payload_resp(m_conn.recv());
    if (payload_resp.deserialize() == false) {
        Client::print_line("Error receiving file list from server!");
        return;
    }
    Client::print_line("Decrypting data...");
    for (std::vector<char> fname_cipher : payload_resp.filenames()) {
        std::vector<char> fname_plain =
            PythonWrapper::get().decrypt(_key, _iv, fname_cipher);
        std::string fname_str(fname_plain.begin(), fname_plain.end());
        files.push_back(fname_str);
    }
    Client::print_line("Files on server:");
    for (std::string fname : files) {
        Client::print_line(" - " + fname);
    }
    std::cin.ignore();
    std::cin.get();
}
void Client::_delete() {
    Client::print("Enter filename to delete: ");
    std::string filename;
    std::cin >> filename;
    Client::print_line("Requesting server to delete file...");
    std::vector<char> filename_cipher = PythonWrapper::get().encrypt(
        _key, _iv, std::vector<char>(filename.begin(), filename.end()));
    PostDeleteRequest payload(filename_cipher, _user);
    m_conn.send(payload.serialize());
}