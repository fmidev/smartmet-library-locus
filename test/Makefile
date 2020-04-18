PROG = $(patsubst %.cpp,%,$(wildcard *.cpp))
# OBS = $(wildcard ../obj/*.o)

DATABASE_HOST = $(shell test "$$CI" = "true" && echo "localhost" || echo "smartmet-test")
DATABASE_PORT = $(shell test "$$CI" = "true" && echo "5432" || echo "5444")

-include $(HOME)/.smartmet.mk
GCC_DIAG_COLOR ?= always
CXX_STD ?= c++11

MAINFLAGS = -Wall -W -Wno-unused-parameter

EXTRAFLAGS = -Werror -pedantic -Wpointer-arith -Wcast-qual \
	-Wcast-align -Wwrite-strings -Wconversion -Winline \
	-Wctor-dtor-privacy -Wnon-virtual-dtor -Wno-pmf-conversions \
	-Wsign-promo -Wchar-subscripts -Wold-style-cast \
	-Wshadow -Woverloaded-virtual

DIFFICULTFLAGS = -Weffc++ -Wunreachable-code -Wredundant-decls

CC = g++
CFLAGS = -DUNIX -D_REENTRANT -std=$(CXX_STD) -O0 -g $(MAINFLAGS) -DDATABASE_PORT=\"$(DATABASE_PORT)\" -DDATABASE_HOST=\"$(DATABASE_HOST)\"

# Boost 1.69

ifneq "$(wildcard /usr/include/boost169)" ""
  INCLUDES += -I/usr/include/boost169
  LIBS += -L/usr/lib64/boost169
endif

INCLUDES += -I../locus \
	-I/usr/include/smartmet


LIBS += ../libsmartmet-locus.so

all: $(PROG)
clean:
	rm -f $(PROG) *~

.ciprep:
	test "$$CI" != "true" || ( /usr/share/smartmet/test/db/init-and-start.sh && \
		/usr/share/smartmet/test/db/install-test-db.sh drop && rm -f $(PROG) && touch .ciprep )

test: .ciprep $(PROG)
	@echo Running tests:
	@rm -f *.err
	@for prog in $(PROG); do \
		( ./$$prog || touch $$prog.err ) ; \
	done
	@test `find . -name \*.err | wc -l` = "0" || ( echo ; echo "The following tests have errors:" ; \
			for i in *.err ; do echo `basename $$i .err`; done ; rm -f *.err ; false ) 


$(PROG) : % : %.cpp ../libsmartmet-locus.so
	$(CC) $(CFLAGS) -o $@ $@.cpp $(INCLUDES) $(LIBS)

