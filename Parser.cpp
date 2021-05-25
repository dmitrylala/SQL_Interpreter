#include "Parser.hpp"



/*
    ----------------------
        Parser methods
    ----------------------
*/

// static fields initialization
std::vector <std::string> Parser::tables;
std::vector <std::pair <std::string, std::string>> Parser::tables_fields;
std::vector <std::pair <std::string, std::string>> Parser::tables_data;

// constructor
Parser::Parser(int fd_src): is_expr(true), is_inside(false), is_text_field(false),
                            scan(fd_src)
{}

// returns true, if lexem is in { "=", "!=", ">", "<", ">=", "<=" }
bool
Parser::is_relation_op(TypeLex type) const
{
    return (type == LEX_EQ || type == LEX_NEQ || type == LEX_GREATER ||
            type == LEX_LESS || type == LEX_GE || type == LEX_LE);
}

// returns true, if lexem is in { "+", "-", "*", "/", "%" }
bool
Parser::is_arithmetic_op(TypeLex type) const
{
    return (type == LEX_PLUS || type == LEX_MINUS || type == LEX_MULT ||
            type == LEX_DIV || type == LEX_MOD);
}

// returns true, if lexem is in { "AND", "OR", "NOT" }
bool
Parser::is_bool_op(TypeLex type) const
{
    return (type == LEX_OR || type == LEX_AND || type == LEX_NOT);
}

// returns true, if lexem is in { "id" | "num" | "str" }
bool
Parser::is_operand(TypeLex type) const
{
    return (type == LEX_ID || type == LEX_NUM || type == LEX_STR);
}

void
Parser::add_table(const std::string &name)
{
    tables.push_back(name);
}

void
Parser::add_table_field(const std::string &table_name, const std::string &field)
{
    tables_fields.push_back(std::pair <std::string, std::string> (table_name, field));
}

void
Parser::add_table_data(const std::string &table_name, const std::string &data)
{
    tables_data.push_back(std::pair <std::string, std::string> (table_name, data));
}

bool
Parser::is_field_in_table(const std::string &field_name, const std::string &table_name) const
{
    for (auto it = tables_fields.begin(); it < tables_fields.end(); ++it) {
        if (it->first == table_name && it->second == field_name) return true;
    }
    return false;
}

void
Parser::delete_table_info(const std::string &name)
{
    std::vector <std::string> names_to_delete;

    for (auto it = tables_fields.begin(); it < tables_fields.end(); ++it) {
        if (it->first == name) {
            names_to_delete.push_back(it->second);
        }
    }
    for (auto it = tables_fields.begin(); it < tables_fields.end(); ++it) {
        if (it->first == name) {
            tables_fields.erase(it);
        }
    }

    for (auto it = tables_data.begin(); it < tables_data.end(); ++it) {
        if (it->first == name) {
            names_to_delete.push_back(it->second);
        }
    }
    for (auto it = tables_data.begin(); it < tables_data.end(); ++it) {
        if (it->first == name) {
            tables_data.erase(it);
        }
    }

    for (auto it = tables.begin(); it < tables.end(); ++it) {
        if (*it == name) {
            names_to_delete.push_back(*it);
            tables.erase(it);
            break;
        }
    }

    for (auto it = names_to_delete.begin(); it < names_to_delete.end(); ++it) {
        scan.TID.delete_info(*it);
    }
}

// utility
long
Parser::get_index(std::vector <Lex> vector, TypeLex type) const
{
    for (long i = 0; i < (long)vector.size(); ++i) {
        if (vector[i].get_type() == type) return i;
    }
    return -1;
}

void
Parser::get_lex()
{
    cur_lex = scan.get_lex();
    cur_type = cur_lex.get_type();
    cur_val = cur_lex.get_val();
}

Table
Parser::execute(bool &tmp_table)
{
    size_t i = 0;
    tmp_table = false;
    if (poliz[i].get_type() == LEX_CREATE) {
        ++i;
        size_t pos_table = poliz[i++].get_val();
        std::string table_name = scan.TID[pos_table].name;
        scan.TID[pos_table].declare = true;
        scan.TID[pos_table].type = LEX_TABLE_ID;

        TableStruct struct_result;
        while (i != poliz.size()) {
            Lex field = poliz[i++];
            Ident field_ident = scan.TID[field.get_val()];
            TypeLex field_type = poliz[i++].get_type();

            if (field_type == LEX_TEXT) {
                field_ident.val = poliz[i++].get_val();
                field_ident.declare = true;
                field_ident.type = LEX_TEXT_FIELD;
                struct_result.add_text(field_ident.name, field_ident.val);
            } else if (field_type == LEX_LONG) {
                field_ident.declare = true;
                field_ident.type = LEX_LONG_FIELD;
                struct_result.add_long(field_ident.name);
            }
            add_table_field(table_name, field_ident.name);
            scan.TID[field.get_val()] = field_ident;
        }
        struct_result.set_name(table_name);
        add_table(table_name);
        return Table(struct_result);
    } else if (poliz[i].get_type() == LEX_INSERT) {
        ++i;
        Lex table_lex = poliz[i++];
        Ident table_ident = scan.TID[table_lex.get_val()];
        CHECK_DECLARE(table_ident, table_lex);
        Table table(table_ident.name);
        Record record = table.get_record(table.size());

        size_t j = 0;
        while (i != poliz.size()) {
            if (j >= record.size()) throw SizeError(j + poliz.size() - i);
            Lex field = poliz[i++];
            if (field.get_type() == LEX_STR) {
                CHECK_TEXT_SIZE(scan.TID[field.get_val()].name, record.get_field(j));
                record.get_field(j)->text() = scan.TID[field.get_val()].name;
                add_table_data(table_ident.name, scan.TID[field.get_val()].name);
            } else if (field.get_type() == LEX_NUM) {
                record.get_field(j)->num() = field.get_val();
            }
            ++j;
        }
        if (j != record.size()) throw SizeError(j);
        table.add(record);
        return table;
    } else if (poliz[i].get_type() == LEX_DROP) {
        ++i;
        Lex table_lex = poliz[i++];
        Ident table_ident = scan.TID[table_lex.get_val()];
        CHECK_DECLARE(table_ident, table_lex);
        Table table(table_ident.name);
        table.drop();
        delete_table_info(table_ident.name);

        TableStruct empty_struct;
        tmp_table = true;
        return Table(empty_struct);
    } else if (poliz[i].get_type() == LEX_UPDATE) {
        ++i;
        Lex table_lex = poliz[i++];
        Ident table_ident = scan.TID[table_lex.get_val()];
        CHECK_DECLARE(table_ident, table_lex);
        Table table(table_ident.name);

        long pos_start = i;
        Lex field_lex = poliz[pos_start];
        Ident field_ident = scan.TID[field_lex.get_val()];
        CHECK_DECLARE(field_ident, field_lex);
        CHECK_FIELD_IN_TABLE(field_ident.name, table_ident.name);

        // let's calculate WHERE-clause first
        std::vector <size_t> records_nums;
        calc_where_clause(records_nums, table);
        long pos_where = get_index(poliz, LEX_WHERE);

        // updating records
        auto record_it = table.begin();
        for (size_t j = 0; j < records_nums.size(); ++j) {
            while ((*record_it).get_id() != records_nums[j]) {
                ++record_it;
            }

            std::stack <Lex> operands;
            calculate(operands, *record_it, pos_start, pos_where);
            Lex result = operands.top();
            operands.pop();

            // result analysis
            if (field_ident.type == LEX_LONG_FIELD) {
                if (result.get_type() == LEX_NUM) {
                    (*record_it).get_field(field_ident.name)->num() = result.get_val();
                } else if (result.get_type() == LEX_ID) {
                    Ident result_ident = scan.TID[result.get_val()];
                    CHECK_DECLARE(result_ident, result);
                    CHECK_IDENT_TYPE(result_ident, result, LEX_LONG_FIELD);
                    CHECK_FIELD_IN_TABLE(result_ident.name, table_ident.name);

                    (*record_it).get_field(field_ident.name)->num() =
                                    (*record_it).get_field(result_ident.name)->num();
                } else {
                    throw ParameterError <Lex> (result, WRONG_TYPE);
                }
            } else if (field_ident.type == LEX_TEXT_FIELD) {
                if (result.get_type() == LEX_ID) {
                    Ident result_ident = scan.TID[result.get_val()];
                    CHECK_DECLARE(result_ident, result);
                    CHECK_IDENT_TYPE(result_ident, result, LEX_TEXT_FIELD);
                    CHECK_FIELD_IN_TABLE(result_ident.name, table_ident.name);

                    CHECK_TEXT_SIZE((*record_it).get_field(result_ident.name)->text(),
                                    (*record_it).get_field(field_ident.name));

                    (*record_it).get_field(field_ident.name)->text() =
                                            (*record_it).get_field(result_ident.name)->text();
                } else if (result.get_type() == LEX_STR) {
                    (*record_it).get_field(field_ident.name)->text() = scan.TID[result.get_val()].name;
                } else {
                    throw ParameterError <Lex> (result, WRONG_TYPE);
                }
            }
            table.update(*record_it);
        }

        return table;
    } else if (poliz[i].get_type() == LEX_SELECT) {
        ++i;
        tmp_table = true;

        TableStruct struct_result;
        std::vector <Ident> fields;
        bool all_fields = false;
        while (poliz[i].get_type() != LEX_FROM) {
            Lex field = poliz[i++];
            if (field.get_type() == LEX_MULT) {
                all_fields = true;
                break;
            } else {
                Ident field_ident = scan.TID[field.get_val()];
                CHECK_DECLARE(field_ident, field);
                if (field_ident.type == LEX_TEXT_FIELD) {
                    struct_result.add_text(field_ident.name, field_ident.val);
                } else if (field_ident.type == LEX_LONG_FIELD) {
                    struct_result.add_long(field_ident.name);
                }
                fields.push_back(field_ident);
            }
        }
        ++i;

        Lex table_lex = poliz[i++];
        Ident table_ident = scan.TID[table_lex.get_val()];
        CHECK_DECLARE(table_ident, table_lex);
        Table table(table_ident.name);

        if (all_fields) {
            struct_result = table.get_struct();
            struct_result.set_name(DEFAULT_FILE_NAME);
            Record rec = table.get_record(table.size());
            for (size_t j = 0; j < rec.size(); ++j) {
                IField *field = rec.get_field(j);
                if (field->type() == IField::TEXT) {
                    fields.push_back(Ident(field->get_name(), true, LEX_TEXT_FIELD));
                } else if (field->type() == IField::LONG) {
                    fields.push_back(Ident(field->get_name(), true, LEX_LONG_FIELD));
                }
            }
        }
        for (auto it = fields.begin(); it < fields.end(); ++it) {
            CHECK_FIELD_IN_TABLE(it->name, table_ident.name);
        }
        Table new_table(struct_result);

        // calculating WHERE-clause
        std::vector <size_t> records_nums;
        calc_where_clause(records_nums, table);

        // adding data to the new table
        auto record_it = table.begin();
        for (size_t j = 0; j < records_nums.size(); ++j) {
            while ((*record_it).get_id() != records_nums[j]) {
                ++record_it;
            }

            Record new_record = new_table.get_record(new_table.size());
            for (auto ident_it = fields.begin(); ident_it < fields.end(); ++ident_it) {
                if (ident_it->type == LEX_TEXT_FIELD) {
                    CHECK_TEXT_SIZE((*record_it).get_field(ident_it->name)->text(),
                                    new_record.get_field(ident_it->name));
                    new_record.get_field(ident_it->name)->text() =
                                    (*record_it).get_field(ident_it->name)->text();
                } else if (ident_it->type == LEX_LONG_FIELD) {
                    new_record.get_field(ident_it->name)->num() =
                                    (*record_it).get_field(ident_it->name)->num();
                }
            }

            new_table.add(new_record);
        }

        return new_table;
    } else if (poliz[i].get_type() == LEX_DELETE) {
        ++i;

        Lex table_lex = poliz[i++];
        Ident table_ident = scan.TID[table_lex.get_val()];
        CHECK_DECLARE(table_ident, table_lex);
        Table table(table_ident.name);

        // calculating WHERE-clause
        std::vector <size_t> records_nums;
        calc_where_clause(records_nums, table);

        // deleting selected records
        for (auto it = records_nums.rbegin(); it != records_nums.rend(); ++it) {
            table.del(table.get_record(*it));
        }
        return table;
    } else {
        throw ParameterError <Lex> (poliz[i], SYNTAX_ERR);
    }
}

void
Parser::calculate(std::stack <Lex> &operands, Record &record, long pos_begin, long pos_end)
{
    ++pos_begin;
    while (pos_begin != pos_end) {
        Lex lex = poliz[pos_begin];
        if (is_operand(lex.get_type())) {
            operands.push(lex);
        } else if (is_relation_op(lex.get_type())) {
            Lex second = operands.top();
            operands.pop();
            Lex first = operands.top();
            operands.pop();

            TypeLex first_type = first.get_type();
            TypeLex second_type = second.get_type();
            long first_val = first.get_val();
            long second_val = second.get_val();

            /*
                Possible cases:
                    id op str
                    id op id
                    id op num

                    str op str
                    str op id
                    str op num

                    num op str
                    num op id
                    num op num
            */

            if (first_type == LEX_ID) {
                Ident first_ident = scan.TID[first_val];

                CHECK_DECLARE(first_ident, first);

                if (second_type == LEX_STR) {

                    CHECK_IDENT_TYPE(first_ident, first, LEX_TEXT_FIELD);

                    std::string field_val = record.get_field(first_ident.name)->text();
                    std::string str_val = scan.TID[second_val].name;
                    bool result = relation_op_calc <std::string> (field_val, str_val,
                                    lex.get_type());
                    operands.push(Lex(LEX_NUM, result));
                } else if (second_type == LEX_ID) {
                    Ident second_ident = scan.TID[second_val];

                    CHECK_DECLARE(second_ident, second);
                    CHECK_IDENT_TYPE(first_ident, first, second_ident.type);

                    if (first_ident.type == LEX_TEXT_FIELD) {
                        std::string first_field_val = record.get_field(first_ident.name)->text();
                        std::string second_field_val = record.get_field(second_ident.name)->text();
                        bool result = relation_op_calc <std::string> (first_field_val,
                                                                second_field_val, lex.get_type());
                        operands.push(Lex(LEX_NUM, result));
                    } else if (first_ident.type == LEX_LONG_FIELD) {
                        long first_field_val = record.get_field(first_ident.name)->num();
                        long second_field_val = record.get_field(second_ident.name)->num();
                        bool result = relation_op_calc <long> (first_field_val, second_field_val,
                                                                lex.get_type());
                        operands.push(Lex(LEX_NUM, result));
                    }
                } else if (second_type == LEX_NUM) {
                    CHECK_IDENT_TYPE(first_ident, first, LEX_LONG_FIELD);

                    long field_val = record.get_field(first_ident.name)->num();
                    bool result = relation_op_calc <long> (field_val, second_val,
                                                                    lex.get_type());
                    operands.push(Lex(LEX_NUM, result));
                }
            } else if (first_type == LEX_STR) {
                std::string str_val = scan.TID[first_val].name;

                if (second_type == LEX_STR) {
                    std::string second_str_val = scan.TID[second_val].name;
                    bool result = relation_op_calc <std::string> (str_val, second_str_val,
                                                                    lex.get_type());
                    operands.push(Lex(LEX_NUM, result));
                } else if (second_type == LEX_ID) {
                    Ident second_ident = scan.TID[second_val];

                    CHECK_DECLARE(second_ident, second);
                    CHECK_IDENT_TYPE(second_ident, second, LEX_TEXT_FIELD);

                    std::string field_val = record.get_field(second_ident.name)->text();
                    bool result = relation_op_calc <std::string> (field_val, str_val,
                                                                lex.get_type());
                    operands.push(Lex(LEX_NUM, result));
                } else if (second_type == LEX_NUM) {
                    throw ParameterError <Lex> (second, WRONG_TYPE);
                }
            } else if (first_type == LEX_NUM) {
                if (second_type == LEX_STR) {
                    throw ParameterError <Lex> (second, WRONG_TYPE);
                } else if (second_type == LEX_ID) {
                    Ident second_ident = scan.TID[second_val];

                    CHECK_DECLARE(second_ident, second);
                    CHECK_IDENT_TYPE(second_ident, second, LEX_LONG_FIELD);

                    long field_val = record.get_field(second_ident.name)->num();
                    bool result = relation_op_calc <long> (field_val, first_val,
                                                            lex.get_type());
                    operands.push(Lex(LEX_NUM, result));
                } else if (second_type == LEX_NUM) {
                    bool result = relation_op_calc <long> (first_val, second_val,
                                                            lex.get_type());
                    operands.push(Lex(LEX_NUM, result));
                }
            }
        } else if (is_arithmetic_op(lex.get_type())) {
            Lex second = operands.top();
            operands.pop();
            Lex first = operands.top();
            operands.pop();

            TypeLex first_type = first.get_type();
            TypeLex second_type = second.get_type();
            long first_val = first.get_val();
            long second_val = second.get_val();

            /*
                possible cases:
                    id op id
                    id op num

                    num op id
                    num op num
            */

            if (first_type == LEX_ID) {
                Ident first_ident = scan.TID[first_val];
                CHECK_DECLARE(first_ident, first);
                CHECK_IDENT_TYPE(first_ident, first, LEX_LONG_FIELD);
                long first_field_val = record.get_field(first_ident.name)->num();

                if (second_type == LEX_ID) {
                    Ident second_ident = scan.TID[second_val];
                    CHECK_DECLARE(second_ident, second);
                    CHECK_IDENT_TYPE(second_ident, second, LEX_LONG_FIELD);

                    long second_field_val = record.get_field(second_ident.name)->num();
                    long result = arithmetic_op_calc(first_field_val, second_field_val,
                                                        lex.get_type());
                    operands.push(Lex(LEX_NUM, result));
                } else if (second_type == LEX_NUM) {
                    long result = arithmetic_op_calc(first_field_val, second_val,
                                                        lex.get_type());
                    operands.push(Lex(LEX_NUM, result));
                }
            } else if (first_type == LEX_NUM) {
                if (second_type == LEX_NUM) {
                    long result = arithmetic_op_calc(first_val, second_val,
                                                        lex.get_type());
                    operands.push(Lex(LEX_NUM, result));
                } else if (second_type == LEX_ID) {
                    Ident second_ident = scan.TID[second_val];
                    CHECK_DECLARE(second_ident, second);
                    CHECK_IDENT_TYPE(second_ident, second, LEX_LONG_FIELD);

                    long second_field_val = record.get_field(second_ident.name)->num();
                    long result = arithmetic_op_calc(first_val, second_field_val,
                                                        lex.get_type());
                    operands.push(Lex(LEX_NUM, result));
                }
            }
        } else if (is_bool_op(lex.get_type())) {
            Lex second = operands.top();
            operands.pop();
            long second_val = second.get_val();

            if (second.get_type() != LEX_NUM) {
                throw ParameterError <Lex> (second, WRONG_TYPE);
            }

            if (lex.get_type() != LEX_NOT) {
                Lex first = operands.top();
                operands.pop();
                long first_val = first.get_val();

                if (first.get_type() != LEX_NUM) {
                    throw ParameterError <Lex> (first, WRONG_TYPE);
                }

                if (lex.get_type() == LEX_OR) {
                    operands.push(Lex(LEX_NUM, first_val || second_val));
                } else if (lex.get_type() == LEX_AND) {
                    operands.push(Lex(LEX_NUM, first_val && second_val));
                }

            } else {
                operands.push(Lex(LEX_NUM, !second_val));
            }
        } else {
            std::string msg = SEMANTICS_ERR;
            msg += ": no such operator or operand";
            throw ParameterError <Lex> (lex, msg);
        }
        ++pos_begin;
    }
}

void
Parser::calc_where_clause(std::vector <size_t> &records_nums, Table &table)
{
    long pos_where = get_index(poliz, LEX_WHERE);
    long pos_like = get_index(poliz, LEX_LIKE);
    long pos_in = get_index(poliz, LEX_IN);
    long pos_all = get_index(poliz, LEX_ALL);

    if (pos_like == -1 && pos_in == -1 && pos_all == -1) {
        // it is bool expression
        for (auto it = table.begin(); it != table.end(); ++it) {
            std::stack <Lex> operands;
            calculate(operands, *it, pos_where, (long)poliz.size());
            bool result = operands.top().get_val();
            operands.pop();
            if (result) records_nums.push_back((*it).get_id());
        }
    } else if (pos_like != -1) {
        // it is expression with LIKE
        bool lexem_not = (poliz[pos_like - 1].get_type() == LEX_NOT);
        size_t i = pos_like;
        ++i;

        Lex field = poliz[pos_like - 1 - (long)lexem_not];
        Ident field_ident = scan.TID[field.get_val()];
        CHECK_DECLARE(field_ident, field);
        CHECK_IDENT_TYPE(field_ident, field, LEX_TEXT_FIELD);
        CHECK_FIELD_IN_TABLE(field_ident.name, table.get_name());

        std::string like_str = scan.TID[poliz[i].get_val()].name;

        // need to convert "%" -> ".*" and "_" -> "."
        // also need to add "\" before regex service symbols
        std::string like_str_regex;
        for (auto it = like_str.begin(); it < like_str.end(); ++it) {
            if (*it == '_') {
                like_str_regex += ".";
            } else if (*it == '%') {
                like_str_regex += ".*";
            } else if (*it == '\\') {
                like_str_regex += "\\";
            } else if (*it == '$' || *it == '.' || *it == '|' || *it == '?' ||
                        *it == '*' || *it == '+' || *it == '(' || *it == ')') {
                like_str_regex += "\\";
                like_str_regex = like_str_regex + *it;
            } else {
                like_str_regex += *it;
            }
        }

        std::regex reg(like_str_regex);
        bool result;
        for (auto it = table.begin(); it != table.end(); ++it) {
            std::string str = (*it).get_field(field_ident.name)->text();
            result = (regex_match(str.begin(), str.end(), reg));
            if (lexem_not) result = !result;
            if (result) records_nums.push_back((*it).get_id());
        }

    } else if (pos_in != -1) {
        // it is expression with IN
        bool lexem_not = (poliz[pos_in - 1].get_type() == LEX_NOT);
        size_t i = pos_in;
        ++i;

        std::set <long> const_list_num;
        std::set <std::string> const_list_text;
        while (i != poliz.size()) {
            if (poliz[i].get_type() == LEX_NUM) {
                const_list_num.emplace(poliz[i].get_val());
            } else if (poliz[i].get_type() == LEX_STR) {
                const_list_text.emplace(scan.TID[poliz[i].get_val()].name);
            }
            ++i;
        }

        bool is_text = const_list_num.empty();

        // calculating expression
        for (auto it = table.begin(); it != table.end(); ++it) {
            std::stack <Lex> operands;
            calculate(operands, *it, pos_where, pos_in - (long)(lexem_not));
            Lex result_calc = operands.top();
            operands.pop();

            // result analysis
            bool result;
            if (!is_text) {
                if (result_calc.get_type() == LEX_NUM) {
                    long val = result_calc.get_val();
                    result = (const_list_num.find(val) != const_list_num.end());
                } else if (result_calc.get_type() == LEX_ID) {
                    Ident result_ident = scan.TID[result_calc.get_val()];
                    CHECK_DECLARE(result_ident, result_calc);
                    CHECK_IDENT_TYPE(result_ident, result_calc, LEX_LONG_FIELD);
                    CHECK_FIELD_IN_TABLE(result_ident.name, table.get_name());

                    long val = (*it).get_field(result_ident.name)->num();
                    result = (const_list_num.find(val) != const_list_num.end());
                } else {
                    throw ParameterError <Lex> (result_calc, WRONG_TYPE);
                }
            } else if (is_text) {
                if (result_calc.get_type() == LEX_ID) {
                    Ident result_ident = scan.TID[result_calc.get_val()];
                    CHECK_DECLARE(result_ident, result_calc);
                    CHECK_IDENT_TYPE(result_ident, result_calc, LEX_TEXT_FIELD);
                    CHECK_FIELD_IN_TABLE(result_ident.name, table.get_name());

                    std::string val = (*it).get_field(result_ident.name)->text();
                    result = (const_list_text.find(val) != const_list_text.end());
                } else if (result_calc.get_type() == LEX_STR) {
                    std::string val = scan.TID[result_calc.get_val()].name;
                    result = (const_list_text.find(val) != const_list_text.end());
                } else {
                    throw ParameterError <Lex> (result_calc, WRONG_TYPE);
                }
            }

            if (lexem_not) result = !result;
            if (result) records_nums.push_back((*it).get_id());
        }
    } else if (pos_all != -1) {
        // it is expression with ALL
        for (auto it = table.begin(); it != table.end(); ++it) {
            records_nums.push_back((*it).get_id());
        }
    }
}

template <typename Type>
bool
Parser::relation_op_calc(const Type &first, const Type &second, TypeLex op_type) const
{
    if (op_type == LEX_EQ) return first == second;
    else if (op_type == LEX_NEQ) return first != second;
    else if (op_type == LEX_GREATER) return first > second;
    else if (op_type == LEX_LESS) return first < second;
    else if (op_type == LEX_GE) return first >= second;
    else if (op_type == LEX_LE) return first <= second;
    else throw ParameterError <std::string> ("Bad operator", SEMANTICS_ERR);
}

long
Parser::arithmetic_op_calc(long first, long second, TypeLex op_type) const
{
    if (op_type == LEX_PLUS) return first + second;
    else if (op_type == LEX_MINUS) return first - second;
    else if (op_type == LEX_MULT) return first * second;
    else if (op_type == LEX_DIV) {
        if (second == 0) throw TypeError(DIVISION_BY_ZERO);
        return first / second;
    } else if (op_type == LEX_MOD) {
        if (second == 0) throw TypeError(DIVISION_BY_ZERO);
        return first % second;
    } else throw ParameterError <std::string> ("Bad operator", SEMANTICS_ERR);
}

void
Parser::parse()
{
    get_lex();
    START();
    CHECK(cur_lex, LEX_FIN);
    while (!operators.empty()) {
        poliz.push_back(operators.top());
        operators.pop();
    }
}

// START -> "INSERT" INSERT | "CREATE" CREATE | "DROP" DROP | "UPDATE" UPDATE |
//     "SELECT" SELECT | "DELETE" DELETE
void
Parser::START()
{
    if (cur_type == LEX_INSERT) {
        poliz.push_back(cur_lex);
        get_lex();
        INSERT();
    } else if (cur_type == LEX_CREATE) {
        poliz.push_back(cur_lex);
        get_lex();
        CREATE();
    } else if (cur_type == LEX_DROP) {
        poliz.push_back(cur_lex);
        get_lex();
        DROP();
    } else if (cur_type == LEX_UPDATE) {
        poliz.push_back(cur_lex);
        get_lex();
        UPDATE();
    } else if (cur_type == LEX_SELECT) {
        poliz.push_back(cur_lex);
        get_lex();
        SELECT();
    } else if (cur_type == LEX_DELETE) {
        poliz.push_back(cur_lex);
        get_lex();
        DELETE();
    } else {
        throw ParameterError <Lex> (cur_lex, SYNTAX_ERR);
    }
}

// INSERT -> "INTO" "table_id" "(" FIELD_VAL { "," FIELD_VAL } ")"
void
Parser::INSERT()
{
    CHECK(cur_lex, LEX_INTO);
    get_lex();
    CHECK(cur_lex, LEX_ID);
    poliz.push_back(cur_lex);
    get_lex();
    CHECK(cur_lex, LEX_LPAREN);
    operators.push(cur_lex);
    get_lex();

    FIELD_VAL();
    while (cur_type == LEX_COMMA) {

        while (operators.top().get_type() != LEX_LPAREN) {
            poliz.push_back(operators.top());
            operators.pop();
        }

        get_lex();
        FIELD_VAL();
    }
    CHECK(cur_lex, LEX_RPAREN);

    while (operators.top().get_type() != LEX_LPAREN) {
        poliz.push_back(operators.top());
        operators.pop();
    }
    operators.pop();

    get_lex();
}

// FIELD_VAL -> "str" | "num"
void
Parser::FIELD_VAL()
{
    if (cur_type == LEX_STR) {
        poliz.push_back(cur_lex);
        get_lex();
    } else if (cur_type == LEX_NUM) {
        poliz.push_back(cur_lex);
        get_lex();
    } else {
        throw ParameterError <Lex> (cur_lex, SYNTAX_ERR);
    }
}

// CREATE -> "TABLE" "table_id" "(" "field_id" FIELD_TYPE { "," "field_id" FIELD_TYPE } ")"
void
Parser::CREATE()
{
    CHECK(cur_lex, LEX_TABLE);
    get_lex();

    CHECK(cur_lex, LEX_ID);
    poliz.push_back(cur_lex);
    get_lex();

    CHECK(cur_lex, LEX_LPAREN);
    operators.push(cur_lex);
    get_lex();

    CHECK(cur_lex, LEX_ID);
    poliz.push_back(cur_lex);
    get_lex();

    FIELD_TYPE();
    while (cur_type == LEX_COMMA) {

        while (operators.top().get_type() != LEX_LPAREN) {
            poliz.push_back(operators.top());
            operators.pop();
        }

        get_lex();

        CHECK(cur_lex, LEX_ID);
        poliz.push_back(cur_lex);
        get_lex();

        FIELD_TYPE();
    }

    CHECK(cur_lex, LEX_RPAREN);

    while (operators.top().get_type() != LEX_LPAREN) {
        poliz.push_back(operators.top());
        operators.pop();
    }
    operators.pop();
    get_lex();
}

// FIELD_TYPE -> "TEXT" "(" "unsigned_int_num" ")" | "LONG"
void
Parser::FIELD_TYPE()
{
    if (cur_type == LEX_TEXT) {
        poliz.push_back(cur_lex);
        get_lex();
        CHECK(cur_lex, LEX_LPAREN);
        operators.push(cur_lex);
        get_lex();
        CHECK(cur_lex, LEX_NUM);
        if (cur_val < 0) throw ParameterError <Lex> (cur_lex, SEMANTICS_ERR);
        poliz.push_back(cur_lex);
        get_lex();

        CHECK(cur_lex, LEX_RPAREN);

        while (operators.top().get_type() != LEX_LPAREN) {
            poliz.push_back(operators.top());
            operators.pop();
        }
        operators.pop();
        get_lex();
    } else if (cur_type == LEX_LONG) {
        poliz.push_back(cur_lex);
        get_lex();
    } else {
        throw ParameterError <Lex> (cur_lex, SYNTAX_ERR);
    }
}

// DROP -> "TABLE" "table_id"
void
Parser::DROP()
{
    CHECK(cur_lex, LEX_TABLE);
    get_lex();
    CHECK(cur_lex, LEX_ID);
    poliz.push_back(cur_lex);
    get_lex();
}

// UPDATE -> "table_id" "SET" "field_id" "=" EXPR "WHERE" WHERE
void
Parser::UPDATE()
{
    CHECK(cur_lex, LEX_ID);
    poliz.push_back(cur_lex);
    get_lex();
    CHECK(cur_lex, LEX_SET);
    get_lex();
    CHECK(cur_lex, LEX_ID);
    poliz.push_back(cur_lex);
    get_lex();
    CHECK(cur_lex, LEX_EQ);
    get_lex();
    EXPR();
    while (!operators.empty()) {
        poliz.push_back(operators.top());
        operators.pop();
    }
    CHECK(cur_lex, LEX_WHERE);
    poliz.push_back(cur_lex);
    get_lex();
    WHERE();
}

// SELECT -> FIELDS "FROM" "table_id" "WHERE" WHERE
void
Parser::SELECT()
{
    FIELDS();
    CHECK(cur_lex, LEX_FROM);
    poliz.push_back(cur_lex);
    get_lex();
    CHECK(cur_lex, LEX_ID);
    poliz.push_back(cur_lex);
    get_lex();
    CHECK(cur_lex, LEX_WHERE);
    poliz.push_back(cur_lex);
    get_lex();
    WHERE();
}

// FIELDS -> "field_id" { "," "field_id" } | "*"
void
Parser::FIELDS()
{
    if (cur_type == LEX_ID) {
        poliz.push_back(cur_lex);
        get_lex();
        while (cur_type == LEX_COMMA) {
            get_lex();
            CHECK(cur_lex, LEX_ID);
            poliz.push_back(cur_lex);
            get_lex();
        }
    } else if (cur_type == LEX_MULT) {
        poliz.push_back(Lex(LEX_MULT, cur_val));
        get_lex();
    } else {
        throw ParameterError <Lex> (cur_lex, SYNTAX_ERR);
    }
}

// DELETE -> "FROM" "table_id" "WHERE" WHERE
void
Parser::DELETE()
{
    CHECK(cur_lex, LEX_FROM);
    get_lex();
    CHECK(cur_lex, LEX_ID);
    poliz.push_back(cur_lex);
    get_lex();
    CHECK(cur_lex, LEX_WHERE);
    poliz.push_back(cur_lex);
    get_lex();
    WHERE();
}

// WHERE -> EXPR_BOOL [<if is_expr==true>[ "NOT" ] OPTION ] | "ALL"
void
Parser::WHERE()
{
    if (cur_type == LEX_ALL) {
        poliz.push_back(cur_lex);
        get_lex();
    } else {
        EXPR_BOOL();
        if (is_expr) {
            if (cur_type == LEX_NOT) {
                while (!operators.empty()) {
                    poliz.push_back(operators.top());
                    operators.pop();
                }
                poliz.push_back(cur_lex);
                get_lex();
            }
            OPTION();
        }
    }
}

// OPTION -> "IN" "(" CONST_LIST ")" | <if is_text_field==true> "LIKE" "str"
void
Parser::OPTION()
{
    if (cur_type == LEX_IN) {
        while (!operators.empty()) {
            poliz.push_back(operators.top());
            operators.pop();
        }
        poliz.push_back(cur_lex);
        get_lex();
        CHECK(cur_lex, LEX_LPAREN);
        operators.push(cur_lex);
        get_lex();
        CONST_LIST();
        CHECK(cur_lex, LEX_RPAREN);
        while (operators.top().get_type() != LEX_LPAREN) {
            poliz.push_back(operators.top());
            operators.pop();
        }
        operators.pop();
        get_lex();
    } else if (is_text_field && cur_type == LEX_LIKE) {
        poliz.push_back(cur_lex);
        get_lex();
        CHECK(cur_lex, LEX_STR);
        poliz.push_back(cur_lex);
        get_lex();
    } else {
        throw ParameterError <Lex> (cur_lex, SYNTAX_ERR);
    }
}

// CONST_LIST -> "str" { ","  "str" } | "num" { "," "num" }
void
Parser::CONST_LIST()
{
    if (cur_type == LEX_STR) {
        poliz.push_back(cur_lex);
        get_lex();
        while (cur_type == LEX_COMMA) {

            while (operators.top().get_type() != LEX_LPAREN) {
                poliz.push_back(operators.top());
                operators.pop();
            }

            get_lex();
            CHECK(cur_lex, LEX_STR);
            poliz.push_back(cur_lex);
            get_lex();
        }
    } else if (cur_type == LEX_NUM) {
        poliz.push_back(cur_lex);
        get_lex();
        while (cur_type == LEX_COMMA) {
            while (operators.top().get_type() != LEX_LPAREN) {
                poliz.push_back(operators.top());
                operators.pop();
            }
            get_lex();
            CHECK(cur_lex, LEX_NUM);
            poliz.push_back(cur_lex);
            get_lex();
        }
    } else {
        throw ParameterError <Lex> (cur_lex, SYNTAX_ERR);
    }
}

// EXPR_BOOL -> BOOL_TERM { "OR" <is_expr=false> BOOL_TERM }
void
Parser::EXPR_BOOL()
{
    BOOL_TERM();
    while (cur_type == LEX_OR) {
        is_expr = false;
        while (!operators.empty() && (operators.top().get_type() == LEX_NOT ||
                operators.top().get_type() == LEX_AND ||
                operators.top().get_type() == LEX_OR)) {
            poliz.push_back(operators.top());
            operators.pop();
        }
        operators.push(cur_lex);
        get_lex();
        BOOL_TERM();
    }
}

// BOOL_TERM -> BOOL_MULT { "AND" <is_expr=false> BOOL_MULT }
void
Parser::BOOL_TERM()
{
    BOOL_MULT();
    while (cur_type == LEX_AND) {
        is_expr = false;
        while (!operators.empty() && (operators.top().get_type() == LEX_NOT ||
                operators.top().get_type() == LEX_AND)) {
            poliz.push_back(operators.top());
            operators.pop();
        }
        operators.push(cur_lex);
        get_lex();
        BOOL_MULT();
    }
}

// BOOL_MULT -> "NOT" <is_expr=false> BOOL_MULT | EXPR [ ("=" | "!=" | "<" | ">" | "<=" | ">=") <is_expr=false> <if is_inside==false throw err> EXPR ] | "(" <is_inside=true> EXPR_BOOL ")" <is_inside=false>
void
Parser::BOOL_MULT()
{
    if (cur_type == LEX_NOT) {
        is_expr = false;
        operators.push(cur_lex);
        get_lex();
        BOOL_MULT();
    } else if (cur_type == LEX_LPAREN) {
        is_inside = true;
        operators.push(cur_lex);
        get_lex();
        EXPR_BOOL();
        CHECK(cur_lex, LEX_RPAREN);
        is_inside = false;
        while (operators.top().get_type() != LEX_LPAREN) {
            poliz.push_back(operators.top());
            operators.pop();
        }
        operators.pop();
        get_lex();
    } else {
        EXPR();
        if (is_relation_op(cur_type)) {
            is_expr = false;
            if (!is_inside) throw ParameterError <Lex> (cur_lex, SYNTAX_ERR);
            operators.push(cur_lex);
            get_lex();
            EXPR();
        }
    }
}

// EXPR -> EXPR_LONG | "field_text_id" <is_text_field=true> | "str"
void
Parser::EXPR()
{
    if (cur_type == LEX_ID && scan.TID[cur_val].type == LEX_TEXT_FIELD) {
        is_text_field = true;
        poliz.push_back(cur_lex);
        get_lex();
    } else if (cur_type == LEX_STR) {
        poliz.push_back(cur_lex);
        get_lex();
    } else {
        EXPR_LONG();
    }
}

// EXPR_LONG -> LONG_TERM { ("+"|"-") LONG_TERM }
void
Parser::EXPR_LONG()
{
    LONG_TERM();
    while (cur_type == LEX_PLUS || cur_type == LEX_MINUS) {
        while (!operators.empty() && (operators.top().get_type() == LEX_MULT ||
                operators.top().get_type() == LEX_DIV ||
                operators.top().get_type() == LEX_MOD ||
                operators.top().get_type() == LEX_PLUS ||
                operators.top().get_type() == LEX_MINUS)) {
            poliz.push_back(operators.top());
            operators.pop();
        }
        operators.push(cur_lex);
        get_lex();
        LONG_TERM();
    }
}

// LONG_TERM -> LONG_MULT { ("*"|"/"|"%") LONG_MULT }
void
Parser::LONG_TERM()
{
    LONG_MULT();
    while (cur_type == LEX_MULT || cur_type == LEX_DIV || cur_type == LEX_MOD) {
        while (!operators.empty() && (operators.top().get_type() == LEX_MULT ||
                operators.top().get_type() == LEX_DIV ||
                operators.top().get_type() == LEX_MOD)) {
            poliz.push_back(operators.top());
            operators.pop();
        }
        operators.push(cur_lex);
        get_lex();
        LONG_MULT();
    }
}

// LONG_MULT -> "field_long_id" | "num" | "(" EXPR_LONG ")"
void
Parser::LONG_MULT()
{
    if (cur_type == LEX_ID && scan.TID[cur_val].type == LEX_LONG_FIELD) {
        poliz.push_back(cur_lex);
        get_lex();
    } else if (cur_type == LEX_NUM) {
        poliz.push_back(cur_lex);
        get_lex();
    } else if (cur_type == LEX_LPAREN) {
        operators.push(cur_lex);
        get_lex();
        EXPR_LONG();
        CHECK(cur_lex, LEX_RPAREN);
        while (operators.top().get_type() != LEX_LPAREN) {
            poliz.push_back(operators.top());
            operators.pop();
        }
        operators.pop();
        get_lex();
    } else {
        throw ParameterError <Lex> (cur_lex, SYNTAX_ERR);
    }
}
