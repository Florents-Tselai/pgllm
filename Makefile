PYTHON ?= python3.11
PYVERSION ?= 3.11

CC ?= /usr/bin/gcc
PYTHON_CONFIG = $(PYTHON)-config
PG_CONFIG ?= pg_config

EXTENSION = pgllm
ΕΧΤVERSION = 0.1.0
MODULE_big = $(EXTENSION)
OBJS = src/pgllm.o
HEADERS = src/pgllm.h

DATA = $(wildcard sql/*--*.sql)

TESTS = $(wildcard test/sql/*.sql)
REGRESS = $(patsubst test/sql/%.sql,%,$(TESTS))
REGRESS_OPTS = --inputdir=test --load-extension=$(EXTENSION)

PG_CFLAGS ?= $(shell $(PYTHON)-config --includes)
PG_LDFLAGS ?= $(shell $(PYTHON)-config --ldflags)
SHLIB_LINK += -lpython$(PYVERSION)

PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

dev: clean all install installcheck