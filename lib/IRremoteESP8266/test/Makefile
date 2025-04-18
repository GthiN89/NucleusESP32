# SYNOPSIS:
#
#   make [all]               - makes everything.
#   make TARGET              - makes the given target.
#   make run                 - makes everything and runs all the tests.
#   make run_tests           - run all tests
#   make run-%               - run specific test file (exclude _test.cpp)
#                              replace % with given test file, eg run-IRsend
#   make clean               - removes all files generated by make.
#   make install-googletest  - install the googletest code suite

# Please tweak the following variable definitions as needed by your
# project, except GTEST_HEADERS, which you can use in your own targets
# but shouldn't modify.

# Points to the root of Google Test, relative to where this file is.
# Remember to tweak this if you move this file.
GTEST_DIR = ../lib/googletest/googletest
GMOCK_DIR = ../lib/googletest/googlemock

# Where to find user code.
USER_DIR = ../src
INCLUDES = -I$(USER_DIR) -I.

# Flags passed to the preprocessor.
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include -isystem $(GMOCK_DIR)/include -DUNIT_TEST -D_IR_LOCALE_=en-AU

# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra -Werror -pthread -std=gnu++11

# Google Test libraries
GTEST_LIBS = gtest.a gtest_main.a gmock.a gmock_main.a

# All tests produced by this Makefile. generated from all *_test.cpp files
TESTS = $(patsubst %.cpp,%,$(wildcard *_test.cpp))

# All Google Test headers.  Usually you shouldn't change this
# definition.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

# All Google Mock headers. Note that all Google Test headers are
# included here too, as they are #included by Google Mock headers.
# Usually you shouldn't change this definition.	
GMOCK_HEADERS = $(GMOCK_DIR)/include/gmock/*.h \
                $(GMOCK_DIR)/include/gmock/internal/*.h \
                $(GTEST_HEADERS)

# House-keeping build targets.

all : $(GTEST_LIBS) $(TESTS)

clean :
	rm -f $(GTEST_LIBS) $(TESTS) *.o

# Build and run all the tests.
run : all
	failed=""; \
	for unittest in $(TESTS); do \
		echo "RUNNING: $${unittest}"; \
	  ./$${unittest} || failed="$${failed} $${unittest}"; \
	done; \
	if [ -n "$${failed}" ]; then \
	  echo "FAIL: :-( :-( Unit test(s)$${failed} failed! :-( :-("; exit 1; \
	else \
	  echo "PASS: \o/ \o/ All unit tests passed. \o/ \o/"; \
	fi

run_tests : run

run-% : %_test
	echo "RUNNING: $*"; \
	./$*_test

install-googletest :
	rm -rf ../lib/googletest
	git clone -b v1.12.x https://github.com/google/googletest.git ../lib/googletest

# Builds gtest.a, gtest_main.a, gmock.a, gmock_main.a.

# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)
GMOCK_SRCS_ = $(GMOCK_DIR)/src/*.cc $(GMOCK_HEADERS)

# All the IR protocol object files.
PROTOCOL_OBJS = $(patsubst %.cpp,%.o,$(wildcard $(USER_DIR)/ir_*.cpp))
PROTOCOLS = $(patsubst $(USER_DIR)/%,%,$(PROTOCOL_OBJS))

# All the IR Protocol header files.
PROTOCOLS_H = $(wildcard $(USER_DIR)/ir_*.h)

# Common object files
COMMON_OBJ = IRutils.o IRtimer.o IRsend.o IRrecv.o IRac.o ir_GlobalCache.o \
             IRtext.o $(PROTOCOLS) gtest_main.a gmock_main.a
# Common dependencies
COMMON_DEPS = $(USER_DIR)/IRrecv.h $(USER_DIR)/IRsend.h $(USER_DIR)/IRtimer.h \
              $(USER_DIR)/IRutils.h $(USER_DIR)/IRremoteESP8266.h \
							$(USER_DIR)/IRac.h $(USER_DIR)/i18n.h $(USER_DIR)/IRtext.h \
							$(PROTOCOLS_H)

# Common test dependencies
COMMON_TEST_DEPS = $(COMMON_DEPS) IRrecv_test.h IRsend_test.h ut_utils.h

# For simplicity and to avoid depending on implementation details of
# Google Mock and Google Test, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Mock and
# Google Test compile fast and for ordinary users their source rarely
# changes.
gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(CXXFLAGS) -c \
        $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(CXXFLAGS) -c \
        $(GTEST_DIR)/src/gtest_main.cc

gmock-all.o : $(GMOCK_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(CXXFLAGS) \
            -c $(GMOCK_DIR)/src/gmock-all.cc

gmock_main.o : $(GMOCK_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(CXXFLAGS) \
            -c $(GMOCK_DIR)/src/gmock_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^

gmock.a : gmock-all.o
	$(AR) $(ARFLAGS) $@ $^

gmock_main.a : gmock_main.o
	$(AR) $(ARFLAGS) $@ $^

# Keep all intermediate files.
.SECONDARY:

# Builds our test.  A test should link with either gtest.a or
# gtest_main.a, depending on whether it defines its own main()
# function.

IRtext.o : $(USER_DIR)/IRtext.cpp $(USER_DIR)/IRtext.h $(USER_DIR)/IRremoteESP8266.h $(USER_DIR)/i18n.h $(USER_DIR)/locale/*.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDES) -c $(USER_DIR)/IRtext.cpp

IRutils.o : $(USER_DIR)/IRutils.cpp $(USER_DIR)/IRutils.h $(USER_DIR)/IRremoteESP8266.h $(USER_DIR)/i18n.h $(USER_DIR)/IRtext.cpp $(USER_DIR)/IRtext.h $(USER_DIR)/locale/*.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDES) -c $(USER_DIR)/IRutils.cpp

IRutils_test.o : IRutils_test.cpp $(COMMON_TEST_DEPS) $(GMOCK_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDES) -c IRutils_test.cpp

IRutils_test : IRutils_test.o ir_NEC.o ir_Nikai.o ir_Toshiba.o IRtext.o $(COMMON_OBJ) $(GTEST_LIBS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o $@

IRtimer.o : $(USER_DIR)/IRtimer.cpp $(USER_DIR)/IRtimer.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(USER_DIR)/IRtimer.cpp

IRsend.o : $(USER_DIR)/IRsend.cpp $(USER_DIR)/IRsend.h $(USER_DIR)/IRremoteESP8266.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(USER_DIR)/IRsend.cpp

IRsend_test.o : IRsend_test.cpp $(USER_DIR)/IRsend.h $(USER_DIR)/IRrecv.h IRsend_test.h $(GMOCK_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDES) -c IRsend_test.cpp

IRrecv.o : $(USER_DIR)/IRrecv.cpp $(USER_DIR)/IRrecv.h $(USER_DIR)/IRremoteESP8266.h $(GMOCK_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(USER_DIR)/IRrecv.cpp

IRrecv_test.o : IRrecv_test.cpp $(USER_DIR)/IRsend.h $(USER_DIR)/IRrecv.h IRsend_test.h $(GMOCK_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDES) -c IRrecv_test.cpp

IRac.o : $(USER_DIR)/IRac.cpp $(USER_DIR)/IRac.h $(COMMON_DEPS) $(GMOCK_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDES) -c $(USER_DIR)/IRac.cpp

IRac_test.o : IRac_test.cpp $(USER_DIR)/IRac.h $(COMMON_DEPS) $(GMOCK_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDES) -c IRac_test.cpp

# new specific targets goes above this line

ir_%.o : $(USER_DIR)/ir_%.h $(USER_DIR)/ir_%.cpp $(COMMON_DEPS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDES) -c $(USER_DIR)/ir_$*.cpp

ir_%.o : $(USER_DIR)/ir_%.cpp $(COMMON_DEPS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDES) -c $(USER_DIR)/ir_$*.cpp

ir_%_test.o : ir_%_test.cpp $(USER_DIR)/ir_$%.h $(COMMON_TEST_DEPS) $(GMOCK_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDES) -c ir_$*_test.cpp

ir_%_test.o : ir_%_test.cpp $(COMMON_TEST_DEPS) $(GMOCK_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDES) -c ir_$*_test.cpp

%_test : $(COMMON_OBJ) %_test.o $(GTEST_LIBS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o $@
