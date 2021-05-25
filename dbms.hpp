#ifndef __DBMS__
#define __DBMS__


#include <vector>
#include <list>
#include <fstream>
#include <cmath>

#include "Exceptions.hpp"



// default number of allocated fields in TableStruct
#define CAPACITY_DFL 30

// default name of created file, file type and gap size in output (to ostream)
#define DEFAULT_FILE_NAME "no_name"
#define FILE_TYPE ".csv"
#define GAP_SIZE 6

// default text and long in empty fields
#define EMPTY_TEXT "(no text)"
#define EMPTY_LONG -1
#define EMPTY_NAME "(empty name)"

// error messages
#define LONG_FROM_TEXT_FIELD_MSG "can't get/set long from field of type TEXT"
#define TEXT_FROM_LONG_FIELD_MSG "can't get/set text from field of type LONG"
#define LEN_FROM_LONG_FIELD_MSG "can't get/set len from field of type LONG"
#define BAD_LEN "bad len of text field"
#define BAD_NAME "bad table/field name"
#define NO_SUCH_FIELD "no field with such name"
#define TOO_BIG_TEXT "too big text for this field"

#define NO_SUCH_FILE_MSG "no such file or directory"

// error macro
#define CHECK_TEXT_SIZE(str, field) \
    do { \
        if (str.size() > field->get_len()) { \
            throw ParameterError <size_t> (str.size(), \
                            field->get_name() + ": " + TOO_BIG_TEXT); \
        } \
    } while (0)


// instantiation of template classes
template class ParameterError <int>;
template class ParameterError <size_t>;


// template class specialization
template<>
class ParameterError<std::string> : public ExceptionBase
{
public:
    ParameterError(std::string param_src, const std::string &msg_src);
    void print(std::ostream &out) const;
    std::string get_msg() const;
private:
    std::string param, msg;
};


// Field interface
class IField
{
public:

    // field's types
    enum TypeField { TEXT, LONG };

    // destructor
    virtual ~IField();

    // getters
    virtual TypeField type() const = 0;
    virtual std::string &text() = 0;
    virtual long &num() = 0;
    virtual std::string get_name() const = 0;
    virtual size_t get_len() const = 0;
    virtual std::string to_string() const = 0;

    // output
    virtual void print(std::ofstream &out) const = 0;
    virtual void print(std::ostream &out) const = 0;
};


class TextField: public IField
{
public:
    // constructor
    TextField(const std::string &name_src, int max_len_src,
                                        const std::string &text_src = EMPTY_TEXT);

    // getters
    TypeField type() const;
    std::string &text();
    long &num();
    std::string get_name() const;
    size_t get_len() const;
    std::string to_string() const;

    // output
    friend std::ofstream &operator<<(std::ofstream &fs_out, const TextField &field);
    void print(std::ofstream &fs_out) const;
    friend std::ostream &operator<<(std::ostream &out, const TextField &field);
    void print(std::ostream &out) const;
private:
    std::string name;
    std::string text_f;
    size_t max_len;
};

class LongField: public IField
{
public:
    // constructor
    LongField(const std::string &name_src, long num_src = EMPTY_LONG);

    // getters
    TypeField type() const;
    std::string &text();
    long &num();
    std::string get_name() const;
    size_t get_len() const;
    std::string to_string() const;

    // output
    friend std::ofstream &operator<<(std::ofstream &fs_out, const LongField &field);
    void print(std::ofstream &fs_out) const;
    friend std::ostream &operator<<(std::ostream &out, const LongField &field);
    void print(std::ostream &out) const;
private:
    std::string name;
    long num_f;
};



class Table;

class TableStruct
{
public:
    friend class Table;

    // constructors and destructor
    TableStruct(const std::string &name_src = DEFAULT_FILE_NAME,
                                                size_t cap_src = CAPACITY_DFL);
    TableStruct(const TableStruct &struct_src);
    ~TableStruct();

    // getters
    size_t size() const;

    // operations with fields
    TableStruct &add_text(const std::string &name_src, int len);
    TableStruct &add_long(const std::string &name_src);
    TableStruct &set_name(const std::string &name_src);

    // other operations
    TableStruct &operator=(const TableStruct &struct_src);
    friend std::ofstream &operator<<(std::ofstream &fs_out, const TableStruct &table_struct);
    friend std::ostream &operator<<(std::ostream &out, const TableStruct &table_struct);
private:
    // service methods
    void copy_struct(const TableStruct &struct_src);
    void delete_struct();

    // data fields
    std::string name;
    IField **fields;
    size_t size_f, cap;
};


class Record
{
public:
    friend class Table;

    // constructors and destructor
    Record(size_t id_src, size_t size_src);
    Record(size_t id_src, IField **fields_src, size_t size_src);
    Record(const Record &record_src);
    ~Record();

    // operator=
    Record &operator=(const Record &record);

    // getters
    IField *get_field(const std::string &name);
    IField *get_field(size_t i);
    size_t size() const;
    size_t get_id() const;
    std::string to_string() const;

    // output
    friend std::ofstream &operator<<(std::ofstream &fs_out, const Record &record);
    friend std::ostream &operator<<(std::ostream &out, const Record &record);
private:
    // service methods
    void copy_record(const Record &record_src);
    void delete_record();

    size_t id;
    IField **fields;
    size_t size_f;
};


class Table
{
    std::string filename;
    TableStruct table_struct;
    std::list <Record> records;
public:

    // record iterator
    class IteratorTable
    {
    public:
        IteratorTable(decltype(records.begin()) iterator_src);

        IteratorTable &operator++ ();
        IteratorTable operator++ (int junk);
        Record &operator* ();
        bool operator!= (const IteratorTable &other) const;
    private:
        decltype(records.begin()) iter;
    };


    // constructors and destructor
    Table(const TableStruct &struct_src);
    Table(const std::string &filename_src);

    //  modifying tables
    void add(const Record &record);
    void update(const Record &record);
    void del(const Record &record);
    void drop() const;

    // getters
    size_t size() const;
    std::string get_name() const;
    TableStruct get_struct() const;
    Record get_record(size_t num) const;
    std::string struct_to_string() const;

    // output to ostream
    friend std::ostream &operator<<(std::ostream &out, const Table table);

    // iterating methods
    IteratorTable begin();
    IteratorTable end();
};




#endif
