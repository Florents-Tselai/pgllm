PYTHON ?= python3.11
PYVERSION ?= 3.11

CC ?= /usr/bin/gcc
PYTHON_CONFIG = $(PYTHON)-config
PG_CONFIG ?= pg_config

EXTENSION = pgllm
MODULE_big = $(EXTENSION)
OBJS = $(EXTENSION).o

DATA = $(wildcard sql/*--*.sql)

TESTS = $(wildcard test/sql/*.sql)
REGRESS = $(patsubst test/sql/%.sql,%,$(TESTS))
REGRESS_OPTS = --inputdir=test --load-extension=$(EXTENSION)

PG_CFLAGS ?= $(shell $(PYTHON)-config --includes)
PG_LDFLAGS ?= $(shell $(PYTHON)-config --ldflags)
SHLIB_LINK += -lpython$(PYVERSION)

PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)