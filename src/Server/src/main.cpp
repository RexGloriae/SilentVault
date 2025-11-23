#include "../include/server.hxx"

int main(int argc, char** argv) {
    Server server(Comms::get());
    server.start();
    return 0;
}