#include "DerivedTests.hpp"


//------------------------------

void
DBMSTest::setup()
{}

void
DBMSTest::execute(int verbose)
{
    if (verbose == 0) cout.setstate(std::ios::failbit);

    try {

        const std::string name = "my_table";

        // creating new table
        TableStruct table_struct;
        table_struct.set_name(name).add_text("First name", 10).add_text("Surname", 15);
        table_struct.add_long("Age").add_text("Phone", 10);
        Table table(table_struct);

        cout << "Table created:" << endl << table << endl;

        // Adding 2 more new records
        Record record = table.get_record(table.size());

        record.get_field("First name")->text() = "Sergey";
        record.get_field("Surname")->text() = "Ivanov";
        record.get_field("Age")->num() = 18;
        record.get_field("Phone")->text() = "145-45-45";
        table.add(record);

        record = table.get_record(table.size());

        record.get_field("First name")->text() = "Alexey";
        record.get_field("Surname")->text() = "Petrov";
        record.get_field("Age")->num() = 20;
        record.get_field("Phone")->text() = "343-65-45";
        table.add(record);

        cout << "Added 2 more records:" << endl << table << endl;

        // modifying second record (numbered from zero)
        record = table.get_record(1);

        record.get_field("First name")->text() = "Andrey";
        record.get_field("Surname")->text() = "Fedorov";
        record.get_field("Age")->num() = 24;
        record.get_field("Phone")->text() = "123-45-18";
        table.update(record);

        cout << "Second record modified:" << endl << table << endl;

        // deleting first record
        record = table.get_record(0);
        table.del(record);

        cout << "First record deleted:" << endl << table << endl;

        // opening existing table
        Table table_opened(name);

        cout << "Opened table:" << endl << table_opened << endl;

        // adding more records
        record = table_opened.get_record(table.size());

        record.get_field("First name")->text() = "Alexey";
        record.get_field("Surname")->text() = "Petrov";
        record.get_field("Age")->num() = 20;
        record.get_field("Phone")->text() = "343-65-45";
        table_opened.add(record);

        record = table_opened.get_record(table.size());

        record.get_field("First name")->text() = "Sergey";
        record.get_field("Surname")->text() = "Ivanov";
        record.get_field("Age")->num() = 19;
        record.get_field("Phone")->text() = "145-45-45";
        table_opened.add(record);

        record = table_opened.get_record(table.size());

        record.get_field("First name")->text() = "Alexander";
        record.get_field("Surname")->text() = "Zaharov";
        record.get_field("Age")->num() = 21;
        record.get_field("Phone")->text() = "450-33-33";
        table_opened.add(record);

        cout << "Opened table with new records:" << endl << table_opened << endl;

        // iterating by records
        cout << "Phones:" << endl;
        for (auto record_it = table_opened.begin(); record_it != table_opened.end(); ++record_it) {
            cout << "Phone: " << (*record_it).get_field("Phone")->text() << endl;
        }

        cout << "Surnames:" << endl;
        for (auto record_it = table_opened.begin(); record_it != table_opened.end(); record_it++) {
            cout << "Surname: " << (*record_it).get_field("Surname")->text() << endl;
        }

        table_opened.drop();


    } catch (const ExceptionBase &err) {
        cerr << err << endl;
    }

    if (verbose == 0) cout.clear();
}

void
DBMSTest::teardown()
{}

//------------------------------

void
ExceptionsDBMSTest::setup()
{}

void
ExceptionsDBMSTest::execute(int verbose)
{
    if (verbose == 0) cout.setstate(std::ios::failbit);

    // empty name - exception will be thrown
    try {
        const std::string name = "";
        TableStruct table_struct(name);
    } catch (const ExceptionBase &err) {
        cout << err << endl;
    }

    TableStruct table_struct;

    // same
    try {
        table_struct.set_name("");
    } catch (const ExceptionBase &err) {
        cout << err << endl;
    }

    // same
    try {
        table_struct.add_text("", 10);
    } catch (const ExceptionBase &err) {
        cout << err << endl;
    }

    // same
    try {
        table_struct.add_long("");
    } catch (const ExceptionBase &err) {
        cout << err << endl;
    }

    // bad len of text field
    try {
        table_struct.add_text("Some text field", -10);
    } catch (const ExceptionBase &err) {
        cout << err << endl;
    }

    const std::string name = "my_table";
    table_struct.set_name(name).add_text("First name", 10).add_text("Surname", 15);
    table_struct.add_long("Age").add_text("Phone", 10);
    Table table(table_struct);

    Record record = table.get_record(table.size());

    // no field with such name
    try {
        record.get_field("Name")->text() = "Alexey";
    } catch (const ExceptionBase &err) {
        cout << err << endl;
    }

    IField *surname = record.get_field("Surname");
    IField *age = record.get_field("Age");

    // TypeError - can't use get_len() method on field of type long
    try {
        cout << age->get_len() << endl;
    } catch (const ExceptionBase &err) {
        cout << err << endl;
    }

    // same - can't use text()
    try {
        cout << age->text() << endl;
    } catch (const ExceptionBase &err) {
        cout << err << endl;
    }

    // same -  can't use long() on field of type text
    try {
        cout << surname->num() << endl;
    } catch (const ExceptionBase &err) {
        cout << err << endl;
    }

    record.get_field("First name")->text() = "Sergeyyyyyyyyyyyyyyyyyyyyy";
    record.get_field("Surname")->text() = "Ivanov";
    record.get_field("Age")->num() = 19;
    record.get_field("Phone")->text() = "145-45-45";

    // error of adding a record: len of text in "First name" field in this record
    // is too big
    try {
        table.add(record);
    } catch (const ExceptionBase &err) {
        cout << err << endl;
    }

    record.get_field("First name")->text() = "Sergey";
    table.add(record);

    // same
    record.get_field("Surname")->text() = "Ivanoooooooooooooooooov";
    try {
        table.update(record);
    } catch (const ExceptionBase &err) {
        cout << err << endl;
    }

    table.drop();

    if (verbose == 0) cout.clear();
}

void
ExceptionsDBMSTest::teardown()
{}

//------------------------------

void
ScannerTest::setup()
{}

void
ScannerTest::execute(int verbose)
{
    if (verbose == 0) cout.setstate(std::ios::failbit);


    const std::string tests[] = {
        "CREATE TABLE Students (First_name TEXT (10), Surname TEXT (15), Age LONG, Phone TEXT (10))",
        "INSERT INTO Students ('Sergey', 'Ivanov', 18, '145-45-45')",
        "SELECT First_name, Surname FROM Students WHERE Age IN (18, 19, 20)",
        "SELECT * FROM Students WHERE Phone LIKE '%-%-45'",
        "SELECT * FROM Students WHERE Phone LIKE '%45%'",
        "SELECT Phone FROM Students WHERE Surname = 'Ivanov'",
        "SELECT Surname FROM Students WHERE ALL",
        "SELECT * FROM Students WHERE First_name LIKE '[ABC][^mno]_x%'",
        "SELECT Surname, Phone FROM Students WHERE (Age >= 19) AND (Surname <= 'M')",
        "UPDATE Students SET Age = Age + 1 WHERE ALL",
        "UPDATE Students SET Age = Age * 10 + 20 % 2 WHERE ALL",
        "DROP TABLE Students",
        ""
    };

    size_t i = 0;
    while (tests[i] != "") {
        cout << "TEST " << i << ": " << endl << tests[i] << endl;

        int fd[2];
        SAFE_SYS(pipe(fd), "Pipe error");

        try {

            Scanner scanner(fd[0]);

            SAFE_SYS(write(fd[1], tests[i].c_str(), tests[i].size()), "Write error");
            close(fd[1]);

            Lex cur_lex;
            while ((cur_lex = scanner.get_lex()).get_type() != LEX_FIN) {
                cout << cur_lex << endl;
            }
            cout << endl;

            close(fd[0]);
        } catch (const ExceptionBase &err) {
            close(fd[0]);
            err.print(cerr);
            cerr << endl;
        }

        ++i;
    }


    if (verbose == 0) cout.clear();
}

void
ScannerTest::teardown()
{}

//------------------------------

void
ParserTest::setup()
{}

void
ParserTest::execute(int verbose)
{
    if (verbose == 0) cout.setstate(std::ios::failbit);


    const std::string tests[] = {
        /*
            CREATE tests
        */
        // correct
        "CREATE TABLE Students (First_name TEXT (10), Surname TEXT (15), Age LONG, Phone TEXT (9))",
        "CREATE TABLE Countries (Name TEXT(20), Language TEXT(15), ID LONG)",

        // syntax errors
        "DO something",
        "CREATE Something",
        "CREATE TABLE (field TEXT (20))",
        "CREATE TABLE Another_table (missing TEXT (20) comma LONG)",
        "CREATE TABLE Empty_struct ()",
        "CREATE TABLE Table (field TEXT (-20))",
        "CREATE TABLE Table (field LONG (20))",

        /*
            INSERT tests
        */
        // correct
        "INSERT INTO Students ('Sergey', 'Ivanov', 18, '145-45-45')",
        "INSERT INTO Students ('Alexey', 'Petrov', 20, '343-65-45')",
        "INSERT INTO Students ('Andrey', 'Fedorov', 23, '123-45-18')",
        "INSERT INTO Students ('Alexander', 'Zaharov', 20, '345-33-33')",
        "INSERT INTO Students ('Andrey', 'Nikolaev', 19, '123-45-45')",
        "INSERT INTO Students ('Ivan', 'Ivanov', 17, '578-53-78')",

        "INSERT INTO Countries ('Russia', 'russian', 1)",
        "INSERT INTO Countries ('Great Britain', 'english', 2)",
        "INSERT INTO Countries ('Germany', 'german', 3)",
        "INSERT INTO Countries ('Italy', 'italian', 4)",
        "INSERT INTO Countries ('Spain', 'spanish', 5)",

        // syntax errors
        "INSERT Students ('anything')",
        "INSERT INTO ('anything')",
        "INSERT INTO Students ('missing' 'comma')",
        "INSERT INTO Students (not_string_and_num)",

        // semantics errors
        "INSERT INTO Magicians ('Sergey', 'Ivanov', 18, '145-45-45')",
        "INSERT INTO Students ('Sergey', 'Ivanov', 18)",
        "INSERT INTO Students ('Sergey', 'Ivanov', 18, '145-45-45', 'male')",
        "INSERT INTO Students (18, 'Ivanov', 'Sergey', '145-45-45')",
        "INSERT INTO Students ('Sergey', 'Ivanov', 'Romanovich', '145-45-45')",
        "INSERT INTO Students ('Sergey Ivanov', 'Ivanov', 18, '145-45-45')",
        "INSERT INTO Students ('Sergey', 'Ivanov', 18, '145-45-45-9999')",
        "INSERT INTO Surname ('Sergey', 'Ivanov', 18, '145-45-45-9999')",

        /*
            SELECT tests
        */
        // correct
        "SELECT First_name, Surname FROM Students WHERE Age IN (17, 18, 19)",
        "SELECT First_name, Surname FROM Students WHERE Age % 3 IN (0, 1)",
        "SELECT First_name, Surname FROM Students WHERE Age % 2 NOT IN (0)",
        "SELECT First_name, Surname FROM Students WHERE First_name IN ('Andrey')",
        "SELECT First_name, Surname FROM Students WHERE Surname IN ('Petrov', 'Fedorov')",
        "SELECT First_name, Surname FROM Students WHERE 'Andrey' IN ('Andrey')",
        "SELECT First_name, Surname FROM Students WHERE 2 IN (0, 2)",
        "SELECT Surname, Phone FROM Students WHERE (Age - 19) IN (0, 1, 2)",
        "SELECT * FROM Students WHERE ALL",
        "SELECT * FROM Students WHERE Phone LIKE '%-%-45'",
        "SELECT * FROM Students WHERE Phone LIKE '%45%'",
        "SELECT * FROM Students WHERE (Surname = 'Ivanov')",
        "SELECT Surname FROM Students WHERE (Age > 20)",
        "SELECT Surname, Age FROM Students WHERE (Age > 20)",
        "SELECT * FROM Students WHERE NOT (Surname = 'Petrov') AND (Age = 20)",
        "SELECT * FROM Students WHERE (Surname != 'Petrov') AND (Age = 20)",
        "SELECT * FROM Students WHERE (Surname = 'Ivanov') OR (Age = 20)",
        "SELECT * FROM Students WHERE (Age > 19) AND (Surname > 'M')",
        "SELECT * FROM Students WHERE ((Age > 19) AND (Surname > 'M'))",
        "SELECT * FROM Students WHERE First_name LIKE '[ABC][^mno]_x%'",

        // syntax errors
        "SELECT FROM",
        "SELECT + FROM haha",
        "SELECT * FROM WHERE",
        "SELECT * FROM Students WHERE Age = 20",
        "SELECT * FROM Students WHERE Surname = 'Ivanov'",
        "SELECT * FROM Students WHERE Surname = 'Ivanov' AND Age = 20",
        "SELECT * FROM Students WHERE (Surname = 'Ivanov') AND",
        "SELECT * FROM Students WHERE (Age > 19) IN (18, 19, 20)",
        "SELECT * FROM Students WHERE (Age > 19) AND (Surname > 'M') IN (18, 19, 20)",
        "SELECT * FROM Students WHERE Age LIKE '%-%-45'",

        // semantics errors
        "SELECT First_name, Surname FROM Students WHERE Surname IN (0, 1)",
        "SELECT First_name, Surname FROM Students WHERE Age IN ('Andrey')",
        "SELECT * FROM Students WHERE Age > 20",
        "SELECT * FROM Students WHERE (Age = 'Ivanov')",

        "SELECT Name, ID FROM Students WHERE ALL",
        "SELECT Surname FROM Countries WHERE ALL",
        "SELECT Name FROM Countries WHERE (Age > 10)",
        "SELECT Language FROM Countries WHERE Age IN (19, 20, 21)",
        "SELECT Name FROM Countries WHERE Surname IN ('Ivanov', 'Petrov')",
        "SELECT Language, ID FROM Countries WHERE (Surname = 'Ivanov')",

        /*
            UPDATE tests
        */
        // correct
        "UPDATE Students SET Age = Age - 1 WHERE ALL",
        "UPDATE Students SET Age = 2 + Age WHERE ALL",
        "UPDATE Students SET Age = ((((Age+1)*2+1-4)*6)/5)-15 WHERE (Age = 21)",
        "UPDATE Students SET Age = 1*Age+1*2+1-4*6/5-15 WHERE NOT (Age = 34)",
        "UPDATE Students SET Age = Age+1 WHERE (Age>0) AND (Age<100) AND (Surname='Ivanov')",
        "UPDATE Students SET Age = Age - 5 WHERE ALL",
        "UPDATE Students SET First_name = 'Vasiliy' WHERE (Surname = 'Petrov')",
        "UPDATE Students SET First_name = Phone WHERE Phone LIKE '%-%-45'",
        "UPDATE Students SET Age = Age WHERE Phone LIKE '%-%-45'",

        // syntax errors
        "UPDATE SET Age = Age + 1 WHERE ALL",
        "UPDATE Students SET = Age + 1 WHERE ALL",
        "UPDATE Students SET Age = WHERE ALL",
        "UPDATE Students SET Age = WHERE",
        "UPDATE Students SET Age = Age + 1 WHERE Age = 20",
        "UPDATE Students SET Age = Age + 1 WHERE Surname = 'Ivanov'",
        "UPDATE Students SET Age = Age + 1 WHERE NOT Age > 15",

        // semantics errors
        "UPDATE Students SET Surname = Surname + 1 WHERE ALL",
        "UPDATE Students SET Age = Surname + 1 WHERE ALL",
        "UPDATE Students SET Age = Surname WHERE ALL",
        "UPDATE Students SET Age = Surname + 1 WHERE ALL",
        "UPDATE Students SET Age = 'Alexey' WHERE ALL",
        "UPDATE Students SET Age = Age / 0 WHERE ALL",
        "UPDATE Students SET Age = Age % 0 WHERE ALL",
        "UPDATE Countries SET Age = ID WHERE ALL",
        "UPDATE Countries SET Language = Surname WHERE ALL",
        "UPDATE Countries SET ID = 1 + Age WHERE ALL",

        /*
            DELETE tests
        */
        // correct
        "SELECT * FROM Students WHERE ALL",
        "DELETE FROM Students WHERE (Age < 0) AND (Surname != 'Ivanov')",
        "DELETE FROM Students WHERE (Age < 0) AND (Surname = 'Ivanov')",
        "DELETE FROM Students WHERE ALL",

        /*
            DROP tests
        */
        // correct
        "DROP TABLE Students",
        "DROP TABLE Countries",

        // semantics errors
        "DROP TABLE Students",
        ""
    };

    size_t i = 0;
    while (tests[i] != "") {
        cout << "TEST " << i << ": " << endl << tests[i] << endl;

        int fd[2];
        SAFE_SYS(pipe(fd), "Pipe error");

        try {

            Parser parser(fd[0]);

            SAFE_SYS(write(fd[1], tests[i].c_str(), tests[i].size()), "Write error");
            close(fd[1]);

            parser.parse();
            std::cout << "Syntax is correct" << std::endl;

            close(fd[0]);

            bool tmp_table;
            Table table = parser.execute(tmp_table);
            cout << "Table:" << endl << table << endl;
            if (tmp_table) table.drop();

        } catch (const ExceptionBase &err) {
            close(fd[0]);
            err.print(cout);
            cout << endl;
        }

        ++i;
    }


    if (verbose == 0) cout.clear();
}

void
ParserTest::teardown()
{}

//------------------------------
