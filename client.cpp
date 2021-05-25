#include <iostream>
#include "sock_wrap.hpp"

#define ADDRESS "my_socket"


int main ()
{

    // creating socket
    UnClientSocket sock(ADDRESS);

    signals_set_handlers();

    try {

        // connecting
        sock.connect();

        // writing messages
        while (true) {
            std::cout << "Enter your request:" << std::endl;
            std::string input_line;
            getline(std::cin, input_line, '\n');
            if (std::cin.eof()) break;

            int send_request = 1;
            sock.write(&send_request, sizeof send_request);
            sock.put_string(input_line);

            size_t num_strings;
            sock.read(&num_strings, sizeof num_strings);

            std::cout << "Answer got:" << std::endl;
            for (size_t i = 0; i < num_strings; ++i) {
                std::cout << sock.get_string();
            }
        }

        sock.shutdown();
        sock.close();
    } catch (const ExceptionBase &err) {
        sock.close();
        std::cerr << err << std::endl;
    }
    return 0;
}
