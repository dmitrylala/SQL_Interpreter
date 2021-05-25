#ifndef __EXCEPTIONS__
#define __EXCEPTIONS__

#include <ostream>
#include <string>
#include <typeinfo>


// errors basic messages
#define TYPE_ERR_MSG "TypeError: "
#define PARAM_ERR_MSG "Parameter error: "
#define SOCK_ERR_MSG "Sockets: "
#define FILE_ERR_MSG "FileError: "
#define SIZE_ERR_MSG "SizeError: "


//------------------------------

// base class
class ExceptionBase
{
public:
    virtual ~ExceptionBase();
    virtual void print(std::ostream &out) const = 0;
    virtual std::string get_msg() const = 0;
    friend std::ostream &operator<< (std::ostream &out, const ExceptionBase &exc);
};

//------------------------------

class TypeError : public ExceptionBase
{
public:
    TypeError(const std::string &msg_src);
    void print(std::ostream &out) const;
    std::string get_msg() const;
private:
    std::string msg;
};

//------------------------------

template <typename ParamType>
class ParameterError : public ExceptionBase
{
public:
    ParameterError(ParamType param_src, const std::string &msg_src);
    void print(std::ostream &out) const;
    std::string get_msg() const;
private:
    ParamType param;
    std::string msg;
};

// template class implemetation
#include "ParameterError.tpp"

//------------------------------

class SockError : public ExceptionBase
{
public:
    SockError(const std::string &msg_src);
    void print(std::ostream &out) const;
    std::string get_msg() const;
private:
    std::string msg;
};

//------------------------------

class FileError : public ExceptionBase
{
public:
    FileError(const std::string &msg_src);
    void print(std::ostream &out) const;
    std::string get_msg() const;
private:
    std::string msg;
};

//------------------------------

class SizeError : public ExceptionBase
{
public:
    SizeError(size_t num_src);
    void print(std::ostream &out) const;
    std::string get_msg() const;
private:
    size_t num;
};

//------------------------------


#endif
