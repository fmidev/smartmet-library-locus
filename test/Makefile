PROG = $(patsubst %.cpp,%,$(wildcard *.cpp))
# OBS = $(wildcard ../obj/*.o)

MAINFLAGS = -Wall -W -Wno-unused-parameter

EXTRAFLAGS = -Werror -pedantic -Wpointer-arith -Wcast-qual \
	-Wcast-align -Wwrite-strings -Wconversion -Winline \
	-Wctor-dtor-privacy -Wnon-virtual-dtor -Wno-pmf-conversions \
	-Wsign-promo -Wchar-subscripts -Wold-style-cast \
	-Wshadow -Woverloaded-virtual

DIFFICULTFLAGS = -Weffc++ -Wunreachable-code -Wredundant-decls

CC = g++
CFLAGS = -DUNIX -D_REENTRANT -std=c++11 -O0 -g $(MAINFLAGS)
INCLUDES = -I ../locus \
	-I /usr/include/smartmet


LIBS = ../libsmartmet-locus.so

all: $(PROG)
clean:
	rm -f $(PROG) *~

test: $(PROG)
	@echo Running tests:
	@rm -f *.err
	@for prog in $(PROG); do \
		( ./$$prog || touch $$prog.err ) ; \
	done
	@test `find . -name \*.err | wc -l` = "0" || ( echo ; echo "The following tests have errors:" ; \
			for i in *.err ; do echo `basename $$i .err`; done ; rm -f *.err ; false ) 


$(PROG) : % : %.cpp ../libsmartmet-locus.so
	$(CC) $(CFLAGS) -o $@ $@.cpp $(INCLUDES) $(LIBS)

