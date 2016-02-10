CXX?=			c++
CXXFLAGS?=		# empty
CXXFLAGS+=		-std=c++11 -Wall -Wextra -pedantic
LDFLAGS?=		# empty

PROGS=			test_set test_list

all: ${PROGS}

test: ${PROGS}
	./test_set
	./test_list

test_set: test.cc dangling_ptr.hh
	${CXX} ${CXXFLAGS} ${LDFLAGS} -DDANGLINGPTR_USE_SET test.cc -o test_set

test_list: test.cc dangling_ptr.hh
	${CXX} ${CXXFLAGS} ${LDFLAGS} -DDANGLINGPTR_USE_LIST test.cc -o test_list

clean:
	rm -f ${PROGS} test_cov test.gcda test.gcno
