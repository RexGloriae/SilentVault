#include "../include/client.hxx"

int main() {
    Comms& conn = Comms::get();
    Client client(conn);
    try {
        client.start_cli(HOSTNAME, PORT);

    } catch (const std::exception& e) {
        std::cout << "Unhandled exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}