PROG = $(patsubst %.cpp,%,$(wildcard *.cpp))
# OBS = $(wildcard ../obj/*.o)

ifdef CI
DATABASE_HOST = "localhost"
DATABASE_PORT = 5444
else
DATABASE_HOST = "smartmet-test"
DATABASE_PORT = 5444
endif

REQUIRES = icu-i18n

include $(shell echo $${PREFIX-/usr})/share/smartmet/devel/makefile.inc

MAINFLAGS = -Wall -W -Wno-unused-parameter $(FLAGS)

CFLAGS = -DUNIX -D_REENTRANT -O0 -g $(MAINFLAGS) -DDATABASE_PORT=\"$(DATABASE_PORT)\" -DDATABASE_HOST=\"$(DATABASE_HOST)\"

INCLUDES += -I../locus

LIBS += ../libsmartmet-locus.so -lpqxx

all: $(PROG)
clean:
	rm -f $(PROG) *~

fminames:
ifdef CI
        @/usr/share/smartmet/test/db/init-and-start.sh && /usr/share/smartmet/test/db/install-test-db.sh
endif

test: fminames $(PROG)
	@echo Running tests:
	@rm -f *.err
	@for prog in $(PROG); do \
		( ./$$prog || touch $$prog.err ) ; \
	done
	@test `find . -name \*.err | wc -l` = "0" || ( echo ; echo "The following tests have errors:" ; \
			for i in *.err ; do echo `basename $$i .err`; done ; rm -f *.err ; false )


$(PROG) : % : %.cpp ../libsmartmet-locus.so
	$(CXX) $(CFLAGS) -o $@ $@.cpp $(INCLUDES) $(LIBS)

