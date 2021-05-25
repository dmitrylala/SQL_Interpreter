#include "Exceptions.hpp"


// virtual destructor of base class
ExceptionBase::~ExceptionBase()
{}

// output
std::ostream &
operator<< (std::ostream &out, const ExceptionBase &exc)
{
    exc.print(out);
    return out;
}


/*
    ---------------------------
        TypeError functions
    ---------------------------
*/

TypeError::TypeError(const std::string &msg_src) : msg(msg_src)
{}

void
TypeError::print(std::ostream &out) const
{
    out << TYPE_ERR_MSG << msg;
}

std::string
TypeError::get_msg() const
{
    return TYPE_ERR_MSG + msg;
}

/*
    ---------------------------
        SockError functions
    ---------------------------
*/

SockError::SockError(const std::string &msg_src) : msg(msg_src)
{}

void
SockError::print(std::ostream &out) const
{
    out << SOCK_ERR_MSG << msg;
}

std::string
SockError::get_msg() const
{
    return SOCK_ERR_MSG + msg;
}

/*
    ---------------------------
        FileError functions
    ---------------------------
*/

FileError::FileError(const std::string &msg_src) : msg(msg_src)
{}

void
FileError::print(std::ostream &out) const
{
    out << FILE_ERR_MSG << msg;
}

std::string
FileError::get_msg() const
{
    return FILE_ERR_MSG + msg;
}

/*
    ---------------------------
        SizeError functions
    ---------------------------
*/

SizeError::SizeError(size_t num_src) : num(num_src)
{}

void
SizeError::print(std::ostream &out) const
{
    out << SIZE_ERR_MSG << num;
}

std::string
SizeError::get_msg() const
{
    return SIZE_ERR_MSG + std::to_string(num);
}
