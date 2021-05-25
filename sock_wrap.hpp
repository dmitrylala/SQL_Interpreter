#ifndef __SOCK_WRAP__
#define __SOCK_WRAP__

#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include "Exceptions.hpp"



#define SAFE(msg, comm) if ((comm) == -1) { throw SockError(msg); }



// base class for socket addresses
class SocketAddress
{
public:
    SocketAddress();
    virtual ~SocketAddress();

    virtual int get_length() = 0;
    virtual SocketAddress *clone() = 0;
    struct sockaddr *get_struct();
protected:
    struct sockaddr *ptr_addr;
};

class UnSocketAddress: public SocketAddress
{
public:
    UnSocketAddress(const char *socket_name);
    int get_length();
    SocketAddress *clone();
};

// base class for sockets
class BaseSocket
{
public:
    explicit BaseSocket(int sd = -1, SocketAddress *p_addr = NULL);
    virtual ~BaseSocket();
    void shutdown(int option = 2);
    void close();
    void write(void *buf, int len);
    void put_char(int c);
    void put_string(const char *str);
    void put_string(const std::string &str);
    int read(void *buf, int len);
    int get_char();
    std::string get_string();
    int get_socket() const;
protected:
    int sock;
    SocketAddress *addr;
};

class ClientSocket: public BaseSocket
{
public:
    void connect();
};

class ServerSocket: public BaseSocket
{
public:
    BaseSocket *accept();
    void bind();
    void listen(int BackLog);
};

// class for client sockets of AF_UNIX family
class UnClientSocket: public ClientSocket
{
public:
    UnClientSocket(const char *address);
};

// class for server sockets of AF_UNIX family
class UnServerSocket: public ServerSocket
{
public:
    UnServerSocket(const char *address);
};


// signals handling

// termination signals handler
void termination_handler(int s);

// setting signal handlers of SIGINT, SIGQUIT and SIGTERM to
// termination handler and with flag SA_RESTART = 0
void signals_set_handlers();



#endif
