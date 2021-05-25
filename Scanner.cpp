#include "Scanner.hpp"



/*
    ---------------------
        Ident methods
    ---------------------
*/


Ident::Ident(const std::string &name_src, bool declare_src, TypeLex type_src,
            long val_src) : name(name_src), declare(declare_src),
                                        type(type_src), val(val_src)
{}

std::ostream &
operator<<(std::ostream &out, const Ident &ident)
{
    out << "Name: " << ident.name << " declare: " << ident.declare << " type: " <<
    ident.type << " val: " << ident.val;
    return out;
}

/*
    --------------------------
        TableIdent methods
    --------------------------
*/

// constructor and destructor
TableIdent::TableIdent(size_t max_size_src)
{
    max_size = max_size_src;
    idents = new Ident[max_size];
    top = 1;
}

TableIdent::~TableIdent()
{
    delete [] idents;
}

//other
Ident &
TableIdent::operator[] (size_t k)
{
    return idents[k];
}

size_t
TableIdent::put(const std::string &name)
{
    for (size_t j = 1; j < top; ++j) {
        if (name == idents[j].name) {
            return j;
        }
    }

    for (size_t j = 1; j < top; ++j) {
        if (idents[j].type == LEX_NULL) {
            idents[j] = Ident(name);
            return j;
        }
    }

    idents[top] = Ident(name);
    ++top;
    return top - 1;
}

size_t
TableIdent::size() const
{
    return top;
}

void
TableIdent::delete_info(const std::string &name)
{
    for (size_t j = 1; j < top; ++j) {
        if (name == idents[j].name) {
            idents[j] = Ident("", 0, LEX_NULL, 0);
        }
    }
}

std::ostream &
operator<<(std::ostream &out, TableIdent &table)
{
    for (size_t i = 1; i < table.size(); ++i) {
        out << table[i] << std::endl;
    }
    return out;
}


/*
    ---------------------
        Lexem methods
    ---------------------
*/


// table of identifiers
TableIdent Scanner::TID(TID_SIZE);

// constructor
Lex::Lex(TypeLex type_src, long val_src) : type(type_src), val(val_src)
{}

// getters
TypeLex
Lex::get_type() const
{
    return type;
}

long
Lex::get_val() const
{
    return val;
}

// output
std::ostream &
operator<<(std::ostream &out, Lex lexem)
{
    out << "Type: ";
    TypeLex type = lexem.get_type();

    size_t i = 1;
    bool was_found = false;
    while (Scanner::types[i] != LEX_NULL) {
        if (type == Scanner::types[i]) {
            was_found = true;
            break;
        }
        ++i;
    }
    if (!was_found) i = 0;
    out << Scanner::names[i] << ", Text: ";
    if (type == LEX_ID || type == LEX_NUM || type == LEX_STR || type == LEX_TABLE_ID ||
        type == LEX_TEXT_FIELD || type == LEX_LONG_FIELD) {
        if (type == LEX_NUM) out << lexem.get_val();
        else {
            out << Scanner::TID[lexem.get_val()].name;
        }
        return out;
    } else {
        out << Scanner::text[i];
        return out;
    }
}

std::string
Lex::to_string() const
{
    std::string result;
    result += "Type: ";

    size_t i = 1;
    bool was_found = false;
    while (Scanner::types[i] != LEX_NULL) {
        if (type == Scanner::types[i]) {
            was_found = true;
            break;
        }
        ++i;
    }
    if (!was_found) i = 0;
    result += Scanner::names[i] + ", Text: ";
    if (type == LEX_ID || type == LEX_NUM || type == LEX_STR || type == LEX_TABLE_ID ||
        type == LEX_TEXT_FIELD || type == LEX_LONG_FIELD) {
        if (type == LEX_NUM) result += val;
        else {
            result += Scanner::TID[val].name;
        }
        return result;
    } else {
        return result + Scanner::text[i];
    }
}

ParameterError<Lex>::ParameterError(Lex param_src, const std::string &msg_src)
{
    param = param_src;
    msg = msg_src;
}

void
ParameterError<Lex>::print(std::ostream &out) const
{
    out << PARAM_ERR_MSG << msg << ": " << param;
}

std::string
ParameterError<Lex>::get_msg() const
{
    std::string result = PARAM_ERR_MSG + msg;
    result += ": " + param.to_string();
    return result;
}

/*
    ---------------------------
        Scanner methods
    ---------------------------
*/

// constructor
Scanner::Scanner(int fd_src)
{
    fd = fd_src;
    clear();
    start = true;
}

// getting next lexem
Lex
Scanner::get_lex()
{
    size_t pos;
    long number = 0;
    cur_state = HALT;
    if (start == true) {
        gc();
        start = false;
    }

    do {
        if (buf_top == BUF_SIZE) throw ParameterError <char> (cur, NO_SUCH_LEXEM);
        switch (cur_state) {
        case HALT:
            if (isspace(cur)) {
                gc();
            } else if (isdigit(cur)) {
                number = cur - '0';
                gc();
                cur_state = NUM;
            } else if (isalpha(cur) || cur == '_') {
                clear();
                add();
                gc();
                cur_state = ID;
            } else if (cur == '>' || cur == '<' || cur == '=' || cur == '!') {
                clear();
                add();
                gc();
                cur_state = REL;
            } else if (cur == '\'') {
                clear();
                gc();
                cur_state = STR;
            } else if (cur == EOF) {
                return Lex(LEX_FIN, 0);
            } else {
                cur_state = DELIM;
            }
            break;
        case NUM:
            if (isdigit(cur)) {
                number = 10 * number + (cur - '0');
                gc();
            } else {
                return Lex(LEX_NUM, number);
            }
            break;
        case ID:
            if (isalpha(cur) || isdigit(cur) || cur == '_') {
                add();
                gc();
            } else {
                pos = look(buf, TW);
                if (pos != 0) {
                    return Lex(words[pos], pos);
                } else {
                    pos = TID.put(buf);
                    return Lex(LEX_ID, pos);
                }
            }
            break;
        case STR:
            if (cur != '\'') {
                add();
                gc();
            } else {
                gc();
                pos = TID.put(buf);
                return Lex(LEX_STR, pos);
            }
            break;
        case REL:
            if (cur == '>' || cur == '<' || cur == '=' || cur == '!') {
                add();
                gc();
            } else {
                pos = look(buf, TD);
                if (pos != 0) {
                    return Lex(dlms[pos], pos);
                } else {
                    throw ParameterError <char> (cur, NO_SUCH_LEXEM);
                }
            }
            break;
        case DELIM:
            clear();
            add();
            pos = look(buf, TD);
            if (pos != 0) {
                gc();
                return Lex(dlms[pos], pos);
            } else {
                throw ParameterError <char> (cur, NO_SUCH_LEXEM);
            }
            break;
        default:
            throw ParameterError <char> (cur, NO_SUCH_STATE);
        }
    } while (true);
}

// clearing buffer
void
Scanner::clear()
{
    buf_top = 0;
    for (size_t j = 0; j < BUF_SIZE; ++j) {
        buf[j] = '\0';
    }
}

// adding current symbol to buffer
void
Scanner::add()
{
    buf[buf_top] = cur;
    ++buf_top;
}

// searching name from buffer in list of names
size_t
Scanner::look(const std::string &buffer, std::string *list)
{
    size_t i = 1;
    while (!list[i].empty()) {
        if (buffer == list[i]) {
            return i;
        }
        ++i;
    }
    return 0;
}

// getting next symbol
void
Scanner::gc()
{
    cur = 0;
    int got = read(fd, &cur, sizeof(char));
    if (got == 0) cur = EOF;
}

// tables

std::string Scanner::TW[] =
{
    "",

    // main commands
    "CREATE",
    "SELECT",
    "INSERT",
    "UPDATE",
    "DELETE",
    "DROP",

    // other commands and clauses
    "FROM",
    "INTO",
    "SET",
    "WHERE",
    "LIKE",
    "ALL",

    // bool
    "IN",
    "NOT",
    "OR",
    "AND",

    // words
    "TABLE",
    "TEXT",
    "LONG",

    ""
};

TypeLex Scanner::words[] =
{
    LEX_NULL,

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

    // words
    LEX_TABLE,
    LEX_TEXT,
    LEX_LONG,

    LEX_NULL
};

std::string Scanner::TD[] =
{
    "",

    "(",
    ")",
    ",",
    "=",
    "!=",
    ">",
    "<",
    ">=",
    "<=",
    "+",
    "-",
    "*",
    "/",
    "%",
    "EOF",

    ""
};

TypeLex Scanner::dlms[] =
{
    LEX_NULL,

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

    LEX_NULL
};

std::string Scanner::names[] =
{
    "LEX_NULL",

    // main commands
    "LEX_CREATE",
    "LEX_SELECT",
    "LEX_INSERT",
    "LEX_UPDATE",
    "LEX_DELETE",
    "LEX_DROP",

    // other commands and clauses
    "LEX_FROM",
    "LEX_INTO",
    "LEX_SET",
    "LEX_WHERE",
    "LEX_LIKE",
    "LEX_ALL",

    // bool
    "LEX_IN",
    "LEX_NOT",
    "LEX_OR",
    "LEX_AND",

    // delimiters
    "LEX_LPAREN",
    "LEX_RPAREN",
    "LEX_COMMA",
    "LEX_EQ",
    "LEX_NEQ",
    "LEX_GREATER",
    "LEX_LESS",
    "LEX_GE",
    "LEX_LE",
    "LEX_PLUS",
    "LEX_MINUS",
    "LEX_MULT",
    "LEX_DIV",
    "LEX_MOD",
    "LEX_FIN",

    // words and other
    "LEX_TABLE",
    "LEX_TEXT",
    "LEX_LONG",

    "LEX_ID",
    "LEX_NUM",
    "LEX_STR",
    "LEX_TABLE_ID",
    "LEX_TEXT_FIELD",
    "LEX_LONG_FIELD",

    "LEX_NULL"
};

TypeLex Scanner::types[] =
{
    LEX_NULL,

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
    LEX_TABLE_ID,
    LEX_TEXT_FIELD,
    LEX_LONG_FIELD,

    LEX_NULL
};

std::string Scanner::text[] =
{
    "",

    // main commands
    "CREATE",
    "SELECT",
    "INSERT",
    "UPDATE",
    "DELETE",
    "DROP",

    // other commands and clauses
    "FROM",
    "INTO",
    "SET",
    "WHERE",
    "LIKE",
    "ALL",

    // bool
    "IN",
    "NOT",
    "OR",
    "AND",

    // delimiters
    "(",
    ")",
    ",",
    "=",
    "!=",
    ">",
    "<",
    ">=",
    "<=",
    "+",
    "-",
    "*",
    "/",
    "%",
    "EOF",

    // words and other
    "TABLE",
    "TEXT",
    "LONG",

    "",
    "",
    "",
    "",
    "",
    "",

    ""
};
