#include "sock_wrap.hpp"



/*
    ---------------------------------
        Socket address base class
    ---------------------------------
*/


SocketAddress::SocketAddress() : ptr_addr(NULL)
{}

SocketAddress::~SocketAddress()
{
    delete ptr_addr;
}

struct sockaddr *
SocketAddress::get_struct()
{
    return ptr_addr;
}


/*
    -------------------------------------
        Socket address AF_UNIX family
    -------------------------------------
*/


UnSocketAddress::UnSocketAddress(const char *socket_name)
{
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, socket_name);
    ptr_addr = (struct sockaddr *)new struct sockaddr_un(addr);
}

int
UnSocketAddress::get_length()
{
    return sizeof *ptr_addr;
}

SocketAddress *
UnSocketAddress::clone()
{
    return new UnSocketAddress(((struct sockaddr_un *)ptr_addr)->sun_path);
}


/*
    -------------------------
        Socket base class
    -------------------------
*/


BaseSocket::BaseSocket(int sock_src, SocketAddress *addr_src) : sock(sock_src), addr(NULL)
{}

BaseSocket::~BaseSocket()
{
    delete addr;
}

void
BaseSocket::shutdown(int option)
{
    SAFE("Shutdown", ::shutdown(sock, option));
}

void
BaseSocket::close()
{
    ::close(sock);
}

void
BaseSocket::write(void *buf, int len)
{
    SAFE("Write", ::write(sock, buf, len));
}

void
BaseSocket::put_char(int c)
{
    SAFE("Write", ::write(sock, &c, sizeof c));
}

void
BaseSocket::put_string(const char *str)
{
    SAFE("Write", ::write(sock, &str, strlen(str) + 1));
}

void
BaseSocket::put_string(const std::string &str)
{
    int len = str.size();
    write(&len, sizeof len);
    SAFE("Write", ::write(sock, str.c_str(), str.size()));
}

int
BaseSocket::read(void *buf, int len)
{
    return ::read(sock, buf, len);
}

int
BaseSocket::get_char()
{
    int c;
    SAFE("Read", ::read(sock, &c, sizeof c));
    return c;
}

std::string
BaseSocket::get_string()
{
    int len;
    int got = read(&len, sizeof len);
    if (got <= 0) throw SockError("Read");

    std::string output;
    output.resize(len);

    int bytes_received = ::read(sock, &output[0], len);
    if (bytes_received < 0) {
        perror("Failed to read data from socket.\n");
        return "";
    }

    output[bytes_received] = 0;
    return output;
}

int
BaseSocket::get_socket() const
{
    return sock;
}

/*
    -------------------------------
        ClientSocket base class
    -------------------------------
*/


void
ClientSocket::connect()
{
    SAFE("Connect", ::connect(sock, addr->get_struct(), addr->get_length()));
}


/*
    -------------------------------
        ServerSocket base class
    -------------------------------
*/


BaseSocket *
ServerSocket::accept()
{
    int client_socket;
    SAFE("Accept", client_socket = ::accept(sock, NULL, NULL));
    return new BaseSocket(client_socket, addr);
}

void
ServerSocket::bind()
{
    SAFE("Bind", ::bind(sock, addr->get_struct(), addr->get_length()));
}

void
ServerSocket::listen(int BackLog)
{
    SAFE("Listen", ::listen(sock, BackLog));
}


/*
    -----------------------------------
        ClientSocket AF_UNIX family
    -----------------------------------
*/


UnClientSocket::UnClientSocket(const char *address)
{
    SAFE("Socket", sock = socket(AF_LOCAL, SOCK_STREAM, 0));
    addr = new UnSocketAddress(address);
}


/*
    -----------------------------------
        ServerSocket AF_UNIX family
    -----------------------------------
*/


UnServerSocket::UnServerSocket(const char *address)
{
    unlink(address);
    SAFE("Socket", sock = socket(AF_LOCAL, SOCK_STREAM, 0));
    addr = new UnSocketAddress(address);
}


// signals handling

// termination signals handler
void
termination_handler(int s)
{}

// setting signal handlers of SIGINT, SIGQUIT and SIGTERM to
// termination handler and with flag SA_RESTART = 0
void
signals_set_handlers()
{
    struct sigaction new_action, old_action;

    new_action.sa_handler = termination_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;

    sigaction(SIGINT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction(SIGINT, &new_action, NULL);

    sigaction(SIGQUIT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction(SIGQUIT, &new_action, NULL);
}
