CXX?=			c++
CXXFLAGS+=		-std=c++11 -Wall -Wextra -pedantic

GCOV?=			gcov
COV_CXXFLAGS?=	-g -O0 --coverage

PROGS=			test_set test_list

all: ${PROGS}

test: ${PROGS}
	./test_set
	./test_list

coverage:
	${CXX} ${CXXFLAGS} ${COV_CXXFLAGS}  test.cc -o test_cov
	./test_cov
	${GCOV} test.cc
	cat danglingptr.hh.gcov

test_set: test.cc danglingptr.hh
	${CXX} ${CXXFLAGS} -DDANGLINGPTR_USE_SET test.cc -o test_set

test_list: test.cc danglingptr.hh
	${CXX} ${CXXFLAGS} -DDANGLINGPTR_USE_LIST test.cc -o test_list

clean:
	rm -f ${PROGS} test_cov test.gcda test.gcno *.gcov
