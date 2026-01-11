#include "../include/client.hxx"

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
    for (int i = 0; i < 100; i++) {
        std::cout << "\n";
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
                   std::cout << "\nError during authentication: " << e.what() << "\n";
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
                   std::cout << "\nError during registration: " << e.what() << "\n";
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
    std::pair<std::vector<char>, std::vector<char>> result =
        wrapper.client_pub_from_pass(pass);
    // send public and salt to server
    PostPublicAndSaltPayload payload(user, result.first, result.second);
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
    std::pair<std::vector<char>, std::vector<char>> pub_salt_pair;
    // request from server pub and salt
    PostPublicAndSaltRequestPayload req_payload(user);
    m_conn.send(req_payload.serialize());
    std::vector<char>    buff = m_conn.recv();
    GetPubAndSaltPayload payload(buff);
    if (payload.deserialize() == false) {
        Client::print("User not found!");
        throw std::runtime_error("user not found");
    }
    pub_salt_pair.first = payload.pub();
    pub_salt_pair.second = payload.salt();

    // R bytes, r int
    std::pair<std::vector<char>, std::vector<char>> result =
        wrapper.client_commit(pass, pub_salt_pair.second);

    PostChallengeRequestPayload chall_req_payload(user);
    m_conn.send(chall_req_payload.serialize());

    buff.clear();
    buff = m_conn.recv();
    GetChallengePayload challenge_payload(buff);
    if (challenge_payload.deserialize() == false) {
        Client::print("Error receiving challenge from server!");
        throw std::runtime_error("error receiving challenge");
    }
    std::vector<char> challenge = challenge_payload.challenge();
    std::vector<char> response = wrapper.client_compute_response(
        pass, result.second, pub_salt_pair.second, challenge);
    // send response
    PostResponsePayload resp_payload(
        pub_salt_pair.first, response, result.first, challenge, user);
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

                break;
            case 2:
                Client::flush_screen();

                break;
            case 3:
                Client::flush_screen();
                break;
            case 4:
                Client::flush_screen();
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