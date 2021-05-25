#ifndef __SCANNER__
#define __SCANNER__


#include <string>
#include <unistd.h>
#include "Exceptions.hpp"



// table of identifiers and buffer of Scanner sizes
#define TID_SIZE 100
#define BUF_SIZE 80

// error messages
#define NO_SUCH_LEXEM "No such lexem"
#define NO_SUCH_STATE "No such state"


// template class for exceptions instantiation
template class ParameterError <char>;



// types of possible lexems
enum TypeLex
{
    // main commands
    LEX_CREATE,
    LEX_SELECT,
    LEX_INSERT,
    LEX_UPDATE,
    LEX_DELETE,
    LEX_DROP,

    // other commands and clauses
    LEX_FROM,
    LEX_INTO,
    LEX_SET,
    LEX_WHERE,
    LEX_LIKE,
    LEX_ALL,

    // bool
    LEX_IN,
    LEX_NOT,
    LEX_OR,
    LEX_AND,

    // delimiters
    LEX_LPAREN,
    LEX_RPAREN,
    LEX_COMMA,
    LEX_EQ,
    LEX_NEQ,
    LEX_GREATER,
    LEX_LESS,
    LEX_GE,
    LEX_LE,
    LEX_PLUS,
    LEX_MINUS,
    LEX_MULT,
    LEX_DIV,
    LEX_MOD,
    LEX_FIN,

    // words and other
    LEX_TABLE,
    LEX_TEXT,
    LEX_LONG,
    LEX_ID,
    LEX_NUM,
    LEX_STR,

    // for fields and tables
    LEX_TABLE_ID,
    LEX_TEXT_FIELD,
    LEX_LONG_FIELD,

    LEX_NULL
};

/*
    -------------------------
        Identifiers class
    -------------------------
*/

struct Ident
{
    Ident(const std::string &name_src = "", bool declare_src = false, TypeLex type_src = LEX_ID,
            long val_src = 0);
    friend std::ostream &operator<<(std::ostream &out, const Ident &ident);

    std::string name;
    bool declare;
    TypeLex type;
    long val;
};

/*
    ------------------------
        TableIdent class
    ------------------------
*/

class TableIdent
{
public:
    // constructor and destructor
    TableIdent(size_t max_size_src);
    ~TableIdent();

    // other
    Ident &operator[] (size_t k);
    size_t put(const std::string &name);
    size_t size() const;
    void delete_info(const std::string &name);
    friend std::ostream &operator<<(std::ostream &out, TableIdent &table);
private:
    Ident *idents;
    size_t max_size, top;
};

/*
    -------------------
        Lexem class
    -------------------
*/

class Lex
{
public:
    Lex(TypeLex type_src = LEX_NULL, long val_src = 0);

    // getters
    TypeLex get_type() const;
    long get_val() const;

    // output
    friend std::ostream &operator<<(std::ostream &out, Lex lexem);
    std::string to_string() const;
private:
    TypeLex type;
    long val;
};

template <>
class ParameterError <Lex> : public ExceptionBase
{
public:
    ParameterError(Lex param_src, const std::string &msg_src);
    void print(std::ostream &out) const;
    std::string get_msg() const;
private:
    Lex param;
    std::string msg;
};

/*
    ---------------------
        Scanner class
    ---------------------
*/

class Scanner
{
public:
    Scanner(int fd_src = 0);
    Lex get_lex();

    // table of identifiers
    static TableIdent TID;
private:
    friend class Lex;
    friend std::ostream &operator<<(std::ostream &out, Lex lexem);

    // table of service words and their lexems
    static std::string TW[];
    static TypeLex words[];

    // table of delimiters and their lexems
    static std::string TD[];
    static TypeLex dlms[];

    // for output
    static std::string names[];
    static TypeLex types[];
    static std::string text[];

    enum State { HALT, NUM, ID, STR, REL, DELIM };
    State cur_state;
    int cur;
    char buf[BUF_SIZE];
    size_t buf_top;
    bool start;

    int fd;

    // service methods
    void clear();
    void add();
    size_t look(const std::string &buffer, std::string *list);
    void gc();
};



#endif
