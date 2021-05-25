#ifndef __PARSER__
#define __PARSER__



#include <set>
#include <stack>
#include <regex>
#include "Scanner.hpp"
#include "dbms.hpp"


// error messages
#define SYNTAX_ERR "Syntax error"
#define SEMANTICS_ERR "Semantics error"
#define NOT_DECLARED "Semantics error: name was not declared"
#define WRONG_TYPE "Semantics error: type error"
#define NO_SUCH_FIELD_IN_TABLE "Semantics error: no such field in table"
#define DIVISION_BY_ZERO "Division by zero"



// error macro
#define SAFE_SYS(call, msg) if ((call) == -1) throw ParameterError <int> (-1, msg);

// semantics checks
#define CHECK(cur_lex, lex_type) \
    do { \
        if (cur_lex.get_type() != lex_type) { \
            throw ParameterError <Lex> (cur_lex, SYNTAX_ERR); \
        } \
    } while (0)

#define CHECK_DECLARE(ident, lex) \
    do { \
        if (!ident.declare) { \
            throw ParameterError <Lex> (lex, NOT_DECLARED); \
        } \
    } while (0)

#define CHECK_IDENT_TYPE(ident, lex, type_expected) \
    do { \
        if (ident.type != type_expected) { \
            throw ParameterError <Lex> (lex, WRONG_TYPE); \
        } \
    } while (0)

#define CHECK_FIELD_IN_TABLE(field_name, table_name) \
    do { \
        if (!is_field_in_table(field_name, table_name)) { \
            throw ParameterError <std::string> (field_name, NO_SUCH_FIELD_IN_TABLE); \
        } \
    } while (0)


/*
    Исходная грамматика:

    START -> INSERT | CREATE | DROP | UPDATE | SELECT | DELETE

    // commands

    INSERT -> "INSERT" "INTO" "id" "(" FIELD_VAL { "," FIELD_VAL } ")"
    FIELD_VAL -> "str" | "num"

    CREATE -> "CREATE" "TABLE" "id" "(" FIELD_DESC { "," FIELD_DESC } ")"
    FIELD_DESC -> "id" FIELD_TYPE
    FIELD_TYPE -> "TEXT" "(" "num" ")" | "LONG"

    DROP -> "DROP" "TABLE" "id"

    UPDATE -> "UPDATE" "id" "SET" "id" "=" EXPR WHERE
    WHERE -> "WHERE" "id" [ "NOT" ] "LIKE" "str" | "WHERE" EXPR [ "NOT" ] "IN" "(" CONST_LIST ")" | "WHERE" EXPR_BOOL | "WHERE" "ALL"
    CONST_LIST -> "str" { ","  "str" } | "num" { "," "num" }

    SELECT -> "SELECT" FIELDS "FROM" "id" WHERE
    FIELDS -> "id" { "," "id" } | "*"

    DELETE -> "DELETE" "FROM" "table_id" WHERE

    // WHERE-clause

    EXPR -> EXPR_LONG | EXPR_TEXT
    EXPR_LONG -> LONG_TERM { ("+"|"-") LONG_TERM }
    LONG_TERM -> LONG_MULT { ("*"|"/"|"%") LONG_MULT }
    LONG_MULT -> LONG_VAL | "(" EXPR_LONG ")"
    LONG_VAL -> "id" | "num"
    EXPR_TEXT -> "id" | "str"
    EXPR_BOOL -> BOOL_TERM { "OR" BOOL_TERM }
    BOOL_TERM -> BOOL_MULT { "AND" BOOL_MULT }
    BOOL_MULT -> "NOT" BOOL_MULT | "(" EXPR_BOOL )" | "(" REL ")"
    REL -> REL_TEXT | REL_LONG
    REL_TEXT -> EXPR_TEXT OP EXPR_TEXT
    REL_LONG -> EXPR_LONG OP EXPR_LONG
    OP -> "=" | ">" | "<" | ">=" | "<=" | "!="



    Эквивалентная грамматика, к которой применим РС-метод:

    START -> "INSERT" INSERT | "CREATE" CREATE | "DROP" DROP | "UPDATE" UPDATE | "SELECT" SELECT | "DELETE" DELETE

    // commands

    INSERT -> "INTO" "table_id" "(" FIELD_VAL { "," FIELD_VAL } ")"
    FIELD_VAL -> "str" | "num"

    CREATE -> "TABLE" "table_id" "(" "field_id" FIELD_TYPE { "," "field_id" FIELD_TYPE } ")"
    FIELD_TYPE -> "TEXT" "(" "unsigned_int_num" ")" | "LONG"

    DROP -> "TABLE" "table_id"

    UPDATE -> "table_id" "SET" "field_id" "=" EXPR "WHERE" WHERE

    SELECT -> FIELDS "FROM" "table_id" "WHERE" WHERE
    FIELDS -> "field_id" { "," "field_id" } | "*"

    DELETE -> "FROM" "table_id" "WHERE" WHERE

    // WHERE-clause and expressions
    WHERE -> EXPR_BOOL [<if is_expr==true>[ "NOT" ] OPTION ] | "ALL"
    OPTION -> "IN" "(" CONST_LIST ")" | <if is_text_field==true> "LIKE" "str"
    CONST_LIST -> "str" { ","  "str" } | "num" { "," "num" }

    EXPR_BOOL -> BOOL_TERM { "OR" <is_expr=false> BOOL_TERM }
    BOOL_TERM -> BOOL_MULT { "AND" <is_expr=false> BOOL_MULT }
    BOOL_MULT -> "NOT" <is_expr=false> BOOL_MULT | EXPR [ ("=" | "!=" | "<" | ">" | "<=" | ">=") <is_expr=false> <if is_inside==false throw err> EXPR ] | "(" <is_inside=true> EXPR_BOOL ")" <is_inside=false>

    EXPR -> EXPR_LONG | "field_text_id" <is_text_field=true> | "str"
    EXPR_LONG -> LONG_TERM { ("+"|"-") LONG_TERM }
    LONG_TERM -> LONG_MULT { ("*"|"/"|"%") LONG_MULT }
    LONG_MULT -> "field_long_id" | "num" | "(" EXPR_LONG ")"
*/



/*
    --------------------
        Parser class
    --------------------
*/

class Parser
{
public:
    Parser(int fd_src = 0);

    // parsing expression and creating poliz
    void parse();

    // poliz execution
    Table execute(bool &tmp_table);
private:
    Lex cur_lex;
    TypeLex cur_type;
    int cur_val;
    bool is_expr, is_inside, is_text_field; // flags for semantic actions in grammar
    Scanner scan;

    std::stack <Lex> operators;
    std::vector <Lex> poliz;

    // tables of existing tables and their fields
    static std::vector <std::string> tables;
    static std::vector <std::pair <std::string, std::string>> tables_fields;
    static std::vector <std::pair <std::string, std::string>> tables_data;

    void get_lex();

    // service semantics methods
    bool is_relation_op(TypeLex type) const;
    bool is_arithmetic_op(TypeLex type) const;
    bool is_bool_op(TypeLex type) const;
    bool is_operand(TypeLex type) const;
    void add_table(const std::string &name);
    void delete_table_info(const std::string &name);
    void add_table_field(const std::string &table_name, const std::string &field);
    void add_table_data(const std::string &table_name, const std::string &data);
    bool is_field_in_table(const std::string &field_name, const std::string &table_name) const;
    long get_index(std::vector <Lex> vector, TypeLex type) const;

    void calculate(std::stack <Lex> &operands, Record &record, long pos_begin, long pos_end);
    void calc_where_clause(std::vector <size_t> &records_nums, Table &table);

    template <typename Type> bool
    relation_op_calc(const Type &first, const Type &second, TypeLex op_type) const;
    long arithmetic_op_calc(long first, long second, TypeLex op_type) const;

    /*
        recursive descent procedures
    */

    void START();

    // insert command
    void INSERT();
    void FIELD_VAL();

    // create command
    void CREATE();
    void FIELD_TYPE();

    // drop command
    void DROP();

    // update command
    void UPDATE();

    // select command
    void SELECT();
    void FIELDS();

    // delete command
    void DELETE();

    // where-clause
    void WHERE();
    void OPTION();
    void CONST_LIST();

    // bool expressions
    void EXPR_BOOL();
    void BOOL_TERM();
    void BOOL_MULT();

    // text or long expressions
    void EXPR();
    void EXPR_LONG();
    void LONG_TERM();
    void LONG_MULT();
};







#endif
