#include "../include/client.hxx"

int main(int argc, char** argv) {
    Comms& conn = Comms::get();
    Client client(conn);
    try {
        if (argc == 1) client.start_cli(HOSTNAME, PORT);
        else {
            // client.start_gui(HOSTNAME, PORT);
        }
    } catch (const std::exception& e) {
        std::cout << "Unhandled exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}