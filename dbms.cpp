#include "dbms.hpp"


/*
    ------------------------------
        ParameterError methods
    ------------------------------
*/


ParameterError<std::string>::ParameterError(std::string param_src, const std::string &msg_src)
{
    param = param_src;
    msg = msg_src;
}

void
ParameterError<std::string>::print(std::ostream &out) const
{
    out << PARAM_ERR_MSG << msg << ": " << param;
}

std::string
ParameterError<std::string>::get_msg() const
{
    std::string result = PARAM_ERR_MSG + msg;
    result += ": " + param;
    return result;
}


/*
    ----------------------
        Fields methods
    ----------------------
*/

/*
    ----------------------
        IFields methods
    ----------------------
*/

// virtual destructor
IField::~IField()
{}


/*
    ----------------------------------------
        TextField and LongField methods
    ----------------------------------------
*/

// constructor
TextField::TextField(const std::string &name_src, int max_len_src,
                                                    const std::string &text_src)
{
    name = name_src;
    max_len = max_len_src;
    text_f = text_src;
}

// getters
IField::TypeField
TextField::type() const
{
    return TEXT;
}

std::string &
TextField::text()
{
    return text_f;
}

long &
TextField::num()
{
    throw TypeError(name + ": " + LONG_FROM_TEXT_FIELD_MSG);
}

std::string
TextField::get_name() const
{
    return name;
}

size_t
TextField::get_len() const
{
    return max_len;
}

std::string
TextField::to_string() const
{
    int len = get_len() + 1 - name.size();
    size_t gap_size = std::max(len, GAP_SIZE);
    std::string gaps(gap_size, ' ');
    return name + gaps;
}

// output
std::ofstream &
operator<<(std::ofstream &fs_out, const TextField &field)
{
    fs_out << field.type() << "," << field.name << "," << field.max_len;
    return fs_out;
}

void
TextField::print(std::ofstream &fs_out) const
{
    fs_out << *this;
}

std::ostream &
operator<<(std::ostream &out, const TextField &field)
{
    int len = field.get_len() + 1 - field.name.size();
    size_t gap_size = std::max(len, GAP_SIZE);
    std::string gaps(gap_size, ' ');
    out << field.name << gaps;
    return out;
}

void
TextField::print(std::ostream &out) const
{
    out << *this;
}

// constructor
LongField::LongField(const std::string &name_src, long num_src): name(name_src),
                                                                    num_f(num_src)
{}

// getters
IField::TypeField
LongField::type() const
{
    return LONG;
}

std::string &
LongField::text()
{
    throw TypeError(name + ": " + TEXT_FROM_LONG_FIELD_MSG);
}

long &
LongField::num()
{
    return num_f;
}

std::string
LongField::get_name() const
{
    return name;
}

size_t
LongField::get_len() const
{
    throw TypeError(name + ": " +  LEN_FROM_LONG_FIELD_MSG);
}

std::string
LongField::to_string() const
{
    int max_long_len = 10;
    size_t gap_size = std::max(max_long_len + 1, GAP_SIZE);
    std::string gaps(gap_size, ' ');
    return name + gaps;
}

// output
std::ofstream &
operator<<(std::ofstream &fs_out, const LongField &field)
{
    fs_out << field.type() << "," << field.name;
    return fs_out;
}

void
LongField::print(std::ofstream &fs_out) const
{
    fs_out << *this;
}

std::ostream &
operator<<(std::ostream &out, const LongField &field)
{
    int max_long_len = 10;
    size_t gap_size = std::max(max_long_len + 1, GAP_SIZE);
    std::string gaps(gap_size, ' ');
    out << field.name << gaps;
    return out;
}

void
LongField::print(std::ostream &out) const
{
    out << *this;
}


/*
    ---------------------------
        TableStruct methods
    ---------------------------
*/

// constructors and destructor
TableStruct::TableStruct(const std::string &name_src, size_t cap_src)
{
    if (name_src.size() == 0) throw ParameterError <std::string> (EMPTY_NAME, BAD_NAME);
    name = name_src;
    size_f = 0;
    cap = cap_src;
    fields = new IField *[cap];
}

TableStruct::TableStruct(const TableStruct &struct_src)
{
    copy_struct(struct_src);
}

TableStruct::~TableStruct()
{
    delete_struct();
}

// getters
size_t
TableStruct::size() const
{
    return size_f;
}

// operations with fields
TableStruct &
TableStruct::add_text(const std::string &name_src, int len)
{
    if (name_src.size() == 0) throw ParameterError <std::string> (EMPTY_NAME, BAD_NAME);
    if (len <= 0) throw ParameterError <int> (len, name_src + ": " + BAD_LEN);
    fields[size_f] = new TextField(name_src, len);
    ++size_f;
    return *this;
}

TableStruct &
TableStruct::add_long(const std::string &name_src)
{
    if (name_src.size() == 0) throw ParameterError <std::string> (EMPTY_NAME, BAD_NAME);
    fields[size_f] = new LongField(name_src);
    ++size_f;
    return *this;
}

TableStruct &
TableStruct::set_name(const std::string &name_src)
{
    if (name_src.size() == 0) throw ParameterError <std::string> (EMPTY_NAME, BAD_NAME);
    name = name_src;
    return *this;
}

// other operations
TableStruct &
TableStruct::operator=(const TableStruct &struct_src)
{
    if (this == &struct_src) return *this;
    delete_struct();
    copy_struct(struct_src);
    return *this;
}

std::ofstream &
operator<<(std::ofstream &fs_out, const TableStruct &table_struct)
{
    for (size_t i = 0; i < table_struct.size(); ++i) {
        table_struct.fields[i]->print(fs_out);
        fs_out << ";";
    }
    fs_out << std::endl;
    return fs_out;
}

std::ostream &
operator<<(std::ostream &out, const TableStruct &table_struct)
{
    for (size_t i = 0; i < table_struct.size(); ++i) {
        table_struct.fields[i]->print(out);
    }
    out << std::endl;
    return out;
}

// service methods
void
TableStruct::copy_struct(const TableStruct &struct_src)
{
    name = struct_src.name;
    size_f = struct_src.size_f;
    cap = struct_src.cap;
    fields = new IField *[cap];
    for (size_t i = 0; i < size_f; ++i) {
        IField *field = struct_src.fields[i];
        if (field->type() == IField::TEXT) {
            fields[i] = new TextField(field->get_name(), field->get_len());
        } else if (field->type() == IField::LONG) {
            fields[i] = new LongField(field->get_name());
        }
    }
}

void
TableStruct::delete_struct()
{
    for (size_t i = 0; i < size_f; ++i) {
        delete fields[i];
    }
    delete [] fields;
}


/*
    ----------------------
        Record methods
    ----------------------
*/

// constructors and destructor
Record::Record(size_t id_src, size_t size_src)
{
    id = id_src;
    size_f = size_src;
    fields = new IField *[size_f];
}

Record::Record(size_t id_src, IField **fields_src, size_t size_src)
{
    id = id_src;
    size_f = size_src;
    fields = new IField *[size_f];

    for (size_t i = 0; i < size_f; ++i) {
        IField *field = fields_src[i];
        if (field->type() == IField::TEXT) {
            fields[i] = new TextField(field->get_name(), field->get_len());
        } else if (field->type() == IField::LONG) {
            fields[i] = new LongField(field->get_name());
        }
    }
}

Record::Record(const Record &record_src)
{
    copy_record(record_src);
}

Record::~Record()
{
    delete_record();
}

// getters
IField *
Record::get_field(const std::string &name)
{
    for (size_t i = 0; i < size(); ++i) {
        if (fields[i]->get_name() == name)
            return fields[i];
    }
    throw ParameterError <std::string> (name, NO_SUCH_FIELD);
}

IField *
Record::get_field(size_t i)
{
    return fields[i];
}

size_t
Record::size() const
{
    return size_f;
}

size_t
Record::get_id() const
{
    return id;
}

std::string
Record::to_string() const
{
    std::string result = "";
    for (size_t i = 0; i < size(); ++i) {
        IField *field = fields[i];
        size_t gap_size;
        if (field->type() == IField::TEXT) {
            int len = field->get_len() + 1;
            int default_gap = GAP_SIZE + field->get_name().size();
            gap_size = std::max(len, default_gap) - (field->text()).size();
            CHECK_TEXT_SIZE((field->text()), field);
            result += field->text();
        } else if (field->type() == IField::LONG) {
            int max_long_len = 10;
            gap_size = std::max(max_long_len, GAP_SIZE) +
                field->get_name().size() - (int)log10(field->num());
            result += std::to_string((field->num()));
        }
        if (i != size() - 1) {
            std::string gaps(gap_size, ' ');
            result += gaps;
        }
    }
    result += "\n";
    return result;
}

// operator=
Record &
Record::operator=(const Record &record)
{
    if (this == &record) return *this;
    delete_record();
    copy_record(record);
    return *this;
}

// output
std::ofstream &
operator<<(std::ofstream &fs_out, const Record &record)
{
    for (size_t i = 0; i < record.size(); ++i) {
        IField *field = record.fields[i];
        if (field->type() == IField::TEXT) {
            CHECK_TEXT_SIZE((field->text()), field);
            fs_out << (field->text()).c_str();
        } else if (field->type() == IField::LONG) {
            fs_out << field->num();
        }
        if (i != record.size() - 1)
            fs_out << ",";
    }
    fs_out << std::endl;
    return fs_out;
}

std::ostream &
operator<<(std::ostream &out, const Record &record)
{
    for (size_t i = 0; i < record.size(); ++i) {
        IField *field = record.fields[i];
        size_t gap_size;
        if (field->type() == IField::TEXT) {
            int len = field->get_len() + 1;
            int default_gap = GAP_SIZE + field->get_name().size();
            gap_size = std::max(len, default_gap) - (field->text()).size();
            CHECK_TEXT_SIZE((field->text()), field);
            out << (field->text()).c_str();
        } else if (field->type() == IField::LONG) {
            int max_long_len = 10;
            int num_len;
            if (field->num() > 0) {
                num_len = (int)log10(field->num());
            } else if (field->num() == 0) {
                num_len = 1;
            } else {
                num_len = (int)log10(abs(field->num())) + 1;
            }
            gap_size = std::max(max_long_len, GAP_SIZE) +
                field->get_name().size() - num_len;
            out << field->num();
        }
        if (i != record.size() - 1) {
            std::string gaps(gap_size, ' ');
            out << gaps;
        }
    }
    out << std::endl;
    return out;
}

// service methods
void
Record::copy_record(const Record &record_src)
{
    id = record_src.id;
    size_f = record_src.size_f;
    fields = new IField *[size_f];

    for (size_t i = 0; i < size_f; ++i) {
        IField *field = record_src.fields[i];
        if (field->type() == IField::TEXT) {
            fields[i] = new TextField(field->get_name(), field->get_len(),
                                                                field->text());
        } else if (field->type() == IField::LONG) {
            fields[i] = new LongField(field->get_name(), field->num());
        }
    }
}

void
Record::delete_record()
{
    for (size_t i = 0; i < size_f; ++i) {
        delete fields[i];
    }
    delete [] fields;
}

/*
    ------------------------
        Table methods
    ------------------------
*/

// constructors and destructor
Table::Table(const TableStruct &struct_src)
{
    table_struct = struct_src;
    filename = table_struct.name;
    std::ofstream fs_out;
    fs_out.open(filename + FILE_TYPE);
    if (!fs_out.good()) throw FileError(NO_SUCH_FILE_MSG);
    fs_out << struct_src;
}

Table::Table(const std::string &filename_src) : table_struct(filename_src)
{
    filename = filename_src;
    std::ifstream fs_in;
    fs_in.open(filename + FILE_TYPE);
    if (!fs_in.good()) throw FileError(NO_SUCH_FILE_MSG);
    std::string struct_string;
    std::getline(fs_in, struct_string, '\n');

    // for parsing
    size_t last_field = 0, next_field = 0;
    size_t last_data = 0, next_data = 0;
    std::string field_data;

    // delimiters
    std::string delimiter_struct = ";";
    std::string delimiter_field = ",";

    // field's data
    int field_type;
    std::string field_name;

    while ((next_field = struct_string.find(delimiter_struct, last_field)) !=
                                                            std::string::npos) {

        // all field's data
        field_data = struct_string.substr(last_field, next_field - last_field);

        // getting field info
        next_data = field_data.find(delimiter_field, last_data);
        field_type = stoi(field_data.substr(last_data, next_data - last_data));
        last_data = next_data + 1;

        next_data = field_data.find(delimiter_field, last_data);
        field_name = field_data.substr(last_data, next_data - last_data);
        last_data = next_data + 1;

        if (field_type == IField::TEXT) {

            // text field's len
            int field_len;

            next_data = field_data.find(delimiter_field, last_data);
            field_len = stoi(field_data.substr(last_data, next_data - last_data));
            last_data = next_data + 1;

            if (field_len < 0) {
                throw ParameterError <int> (field_len, field_name + ": " + BAD_LEN);
            }

            table_struct.fields[table_struct.size_f] = new TextField(field_name, field_len);

        } else if (field_type == IField::LONG) {
            table_struct.fields[table_struct.size_f] = new LongField(field_name);
        }

        ++table_struct.size_f;
        last_field = next_field + 1;
    }

    std::string record_str;
    size_t id = 0;
    last_field = 0;
    next_field = 0;
    while (getline(fs_in, record_str, '\n')) {

        Record record(id, table_struct.size());

        for (size_t j = 0; j < table_struct.size(); ++j) {
            next_field = record_str.find(delimiter_field, last_field);
            field_data = record_str.substr(last_field, next_field - last_field);
            last_field = next_field + 1;

            IField *field = table_struct.fields[j];
            if (field->type() == IField::TEXT) {
                CHECK_TEXT_SIZE(field_data, field);
                record.fields[j] = new TextField(field->get_name(),
                                                    field->get_len(), field_data);
            } else if (field->type() == IField::LONG) {
                record.fields[j] = new LongField(field->get_name(), stol(field_data));
            }
        }

        records.push_back(record);
        ++id;
    }
}

// modifying tables
void
Table::add(const Record &record)
{
    std::ofstream fs_out;
    fs_out.open(filename + FILE_TYPE, std::ios::app);
    if (!fs_out.good()) throw FileError(NO_SUCH_FILE_MSG);
    fs_out << record;
    records.push_back(record);
}

void
Table::update(const Record &record)
{
    // checking if record's text fields fit table_struct
    for (size_t j = 0; j < table_struct.size(); ++j) {
        IField *field = table_struct.fields[j];
        if (field->type() == IField::TEXT) {
            CHECK_TEXT_SIZE((record.fields[j]->text()), field);
        }
    }

    // updating record
    for (auto record_it = records.begin(); record_it != records.end(); ++record_it) {
        if (record_it->id == record.id)
            *record_it = record;
    }

    // deleting old file
    drop();

    // writing all data to new file
    std::ofstream fs_out;
    fs_out.open(filename + FILE_TYPE);
    if (!fs_out.good()) throw FileError(NO_SUCH_FILE_MSG);

    fs_out << table_struct;

    for (auto record_it = records.begin(); record_it != records.end(); ++record_it) {
        fs_out << *record_it;
    }
}

void
Table::del(const Record &record)
{
    // deleting record
    size_t id = record.id;
    for (auto record_it = records.begin(); record_it != records.end(); ++record_it) {
        if (record_it->id == id) {
            records.erase(record_it);
            break;
        }
    }

    for (auto record_it = records.begin(); record_it != records.end(); ++record_it) {
        if (record_it->id >= id) {
            record_it->id -= 1;
        }
    }

    // deleting old file
    drop();

    // writing all data to new file
    std::ofstream fs_out;
    fs_out.open(filename + FILE_TYPE);
    if (!fs_out.good()) throw FileError(NO_SUCH_FILE_MSG);

    fs_out << table_struct;

    for (auto record_it = records.begin(); record_it != records.end(); ++record_it) {
        fs_out << *record_it;
    }
}

void
Table::drop() const
{
    std::remove((filename + FILE_TYPE).c_str());
}

// getters
size_t
Table::size() const
{
    return records.size();
}

std::string
Table::get_name() const
{
    return table_struct.name;
}

TableStruct
Table::get_struct() const
{
    return table_struct;
}

Record
Table::get_record(size_t num) const
{
    for (auto record = records.begin(); record != records.end(); ++record) {
        if (record->id == num) {
            return *record;
        }
    }

    return Record(size(), table_struct.fields, table_struct.size());
}

std::string
Table::struct_to_string() const
{
    std::string result = "";
    for (size_t i = 0; i < table_struct.size(); ++i) {
        result += table_struct.fields[i]->to_string();
    }
    result += "\n";
    return result;
}

// output to ostream
std::ostream &
operator<<(std::ostream &out, const Table table)
{
    std::ifstream fs_out;
    fs_out.open(table.filename + FILE_TYPE);
    if (fs_out.good()) {
        out << table.table_struct;
        for (auto record = table.records.begin(); record != table.records.end(); ++record) {
            out << *record;
        }
    }
    return out;
}

// iterating methods
Table::IteratorTable
Table::begin()
{
    return IteratorTable(records.begin());
}

Table::IteratorTable
Table::end()
{
    return IteratorTable(records.end());
}


/*
    -----------------------------
        IteratorTable methods
    -----------------------------
*/


Table::IteratorTable::IteratorTable(decltype(records.begin()) iterator_src)
{
    iter = iterator_src;
}

Table::IteratorTable &
Table::IteratorTable::operator++ ()
{
    ++iter;
    return *this;
}

Table::IteratorTable
Table::IteratorTable::operator++ (int junk)
{
    IteratorTable new_iter(*this);
    iter++;
    return new_iter;
}

Record &
Table::IteratorTable::operator* ()
{
    return *iter;
}

bool
Table::IteratorTable::operator!= (const IteratorTable &other) const
{
    return iter != other.iter;
}
