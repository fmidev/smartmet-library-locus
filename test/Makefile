PROG = $(patsubst %.cpp,%,$(wildcard *.cpp))
# OBS = $(wildcard ../obj/*.o)

TEST_DB_DIR := $(shell pwd)/tmp-geonames-db

ifdef CI
DATABASE_HOST = "$(TEST_DB_DIR)"
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

LIBS += ../libsmartmet-locus.so -lsmartmet-macgyver -lpqxx

all: $(PROG)
clean:
	rm -f $(PROG) *~
	rm -rf tmp-geonames-db
	rm -f tmp-geonames-db.log

ifdef CI
TEST_PREPARE_TARGETS := start-geonames-db
TEST_FINISH_TARGETS :=  stop-geonames-db
else
TEST_PREPARE_TARGETS := dummy
TEST_FINISH_TARGETS := dummy
endif

test: $(PROG) $(TEST_PREPARE_TARGETS)
	@echo Running tests:
	@rm -f *.err
	@err=; \
	for prog in $(PROG); do \
		( ./$$prog || err="$$err $$prog" ) ; \
	done; \
	$(MAKE) $(TEST_FINISH_TARGETS); \
	test -z "$$err"

$(PROG) : % : %.cpp ../libsmartmet-locus.so
	$(CXX) $(CFLAGS) -o $@ $@.cpp $(INCLUDES) $(LIBS)

geonames-database:
	@-$(MAKE) stop-geonames-db
	rm -rf tmp-geonames-db
	if ! /usr/share/smartmet/test/db/create-local-db.sh $(TEST_DB_DIR) >tmp-geonames-db.log 2>&1 ; then \
	    cat tmp-geonames-db.log; \
	    false; \
	fi

start-geonames-db: geonames-database
	/usr/share/smartmet/test/db/test-db-ctl.sh $(TEST_DB_DIR) start -w

stop-geonames-db:
	-/usr/share/smartmet/test/db/test-db-ctl.sh $(TEST_DB_DIR) stop -w

dummy:
	true
