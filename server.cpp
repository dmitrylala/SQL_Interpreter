#include <iostream>
#include "sock_wrap.hpp"
#include "Parser.hpp"


#define BACKLOG 10
#define ADDRESS "my_socket"


int main()
{
    // creating server socket
    UnServerSocket sock(ADDRESS);

    try {

        sock.bind();
        sock.listen(BACKLOG);

        signals_set_handlers();

        // listening
        while (true) {
            BaseSocket *sock_client = sock.accept();

            int got_request;
            while ((got_request = sock_client->read(&got_request, sizeof got_request))) {
                int fd[2];
                SAFE_SYS(pipe(fd), "Too long request");
                try {
                    std::string request = sock_client->get_string();
                    Parser parser(fd[0]);
                    SAFE_SYS(write(fd[1], request.c_str(), request.size()), "Write to scanner error");
                    close(fd[1]);

                    // parsing
                    parser.parse();
                    close(fd[0]);

                    // calculating result
                    bool tmp_table;
                    Table table = parser.execute(tmp_table);
                    if (tmp_table) table.drop();

                    // sending result to client
                    size_t size = table.size() + 1;
                    sock_client->write(&size, sizeof size);
                    sock_client->put_string(table.struct_to_string());
                    for (auto i = table.begin(); i != table.end(); ++i) {
                        sock_client->put_string((*i).to_string());
                    }
                } catch (const ExceptionBase &err) {
                    std::cerr << err << std::endl;
                    size_t size = 1;
                    sock_client->write(&size, sizeof size);
                    sock_client->put_string(err.get_msg() + "\n");
                }
            }
            delete sock_client;
        }
        sock.shutdown();
        sock.close();
    } catch (const ExceptionBase &err) {
        sock.close();
        std::cerr << err << std::endl;
    }

    return 0;
}
