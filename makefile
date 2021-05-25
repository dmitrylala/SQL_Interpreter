CXX = g++
CXXFLAGS = -g -O2 -ftrapv -fsanitize=address,undefined -Wall -Werror -Wformat-security -Wignored-qualifiers -Winit-self -Wswitch-default -Wfloat-equal -Wshadow -Wpointer-arith -Wtype-limits -Wempty-body -Wmissing-field-initializers -Wcast-qual -Wwrite-strings -Werror=switch-default -std=c++11 -Wlogical-op -Werror=maybe-uninitialized
LINK_FLAGS = -fsanitize=undefined -lm -o



# client and server default compilation

all: client server

client: client.o sock_wrap.o Exceptions.o
	$(CXX) $(CXXFLAGS) $(LINK_FLAGS) $@ $^
server: server.o sock_wrap.o Parser.o Scanner.o dbms.o Exceptions.o
	$(CXX) $(CXXFLAGS) $(LINK_FLAGS) $@ $^
%.o: %.cxx
	$(CXX) $(CXXFLAGS) -c $^ -o $@



# test dbms, Scanner and Parser compilation

test: test_main.o Test.o DerivedTests.o dbms.o Scanner.o Parser.o Exceptions.o
	$(CXX) $(CXXFLAGS) $(LINK_FLAGS) $@ $^
%.o: %.cxx %.hxx
	$(CXX) $(CXXFLAGS) -c $^ -o $@



# test compilation with code coverage

test_cov: test_main_cov.o Test_cov.o DerivedTests_cov.o dbms_cov.o Scanner_cov.o Parser_cov.o Exceptions_cov.o
	$(CXX) --coverage $(CXXFLAGS) $(LINK_FLAGS) $@ $^
test_main_cov.o: test_main.cpp
	$(CXX) --coverage $(CXXFLAGS) -c $^ -o $@
Test_cov.o: Test.cpp
	$(CXX) --coverage $(CXXFLAGS) -c $^ -o $@
DerivedTests_cov.o: DerivedTests.cpp
	$(CXX) --coverage $(CXXFLAGS) -c $^ -o $@
dbms_cov.o: dbms.cpp 
	$(CXX) --coverage $(CXXFLAGS) -c $^ -o $@
Scanner_cov.o: Scanner.cpp
	$(CXX) --coverage $(CXXFLAGS) -c $^ -o $@
Parser_cov.o: Parser.cpp
	$(CXX) --coverage $(CXXFLAGS) -c $^ -o $@
Exceptions_cov.o: Exceptions.cpp
	$(CXX) --coverage $(CXXFLAGS) -c $^ -o $@



test_code_coverage: test_main_cov.gcda Test_cov.gcda DerivedTests_cov.gcda dbms_cov.gcda Scanner_cov.gcda Parser_cov.gcda Exceptions_cov.gcda
	lcov -t "test" --output-file code_coverage.info --capture --directory .
	genhtml --branch-coverage --highlight --legend -o code_coverage code_coverage.info


# cleaning binary and temporary files
clear:
	rm *.o client server my_socket *.csv
clear_test:
	rm *.o test
clear_test_cov:
	rm *.o test_cov *.gcda *.gcno *.info
