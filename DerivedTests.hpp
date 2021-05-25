#ifndef __DERIVED_TESTS__
#define __DERIVED_TESTS__


#include <iostream>
#include "Test.hpp"
#include "Parser.hpp"



using std::cout;
using std::cerr;
using std::endl;


//------------------------------

class DBMSTest : public Test
{
public:
    void setup();
    void execute(int verbose = 0);
    void teardown();
};

//------------------------------

class ExceptionsDBMSTest : public Test
{
public:
    void setup();
    void execute(int verbose = 0);
    void teardown();
};

//------------------------------

class ScannerTest : public Test
{
public:
    void setup();
    void execute(int verbose = 0);
    void teardown();
};

//------------------------------

class ParserTest : public Test
{
public:
    void setup();
    void execute(int verbose = 0);
    void teardown();
};

//------------------------------

#endif
