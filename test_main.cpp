#include <iostream>
#include "DerivedTests.hpp"

#define VERBOSE 0

using std::cout;
using std::cerr;
using std::endl;


int main()
{
    Test *m[] = { new DBMSTest, new ExceptionsDBMSTest, new ScannerTest,
                    new ParserTest, NULL };
    // Test *m[] = { new ParserTest, NULL };
    // Test *m[] = { new ScannerTest, NULL };

    Test **p = m;
    while (*p != NULL) {

        (*p)->setup();

        try {
            (*p)->execute(VERBOSE);
        } catch (...) {
            cerr << "Something wrong" << endl;
            (*p)->teardown();
            break;
        }

        (*p)->teardown();
        ++p;

        if (VERBOSE) cout << endl;
        cout << "Passed" << endl;
        if (VERBOSE) cout << endl;
    }

    p = m;

    while (*p != NULL) {
        delete *p;
        ++p;
    }
}
