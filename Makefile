PYTHON ?= python3.11
PYVERSION ?= $(shell $(PYTHON) -c "from sysconfig import get_config_vars; print(get_config_vars().get('py_version_short'))")

CC ?= /usr/bin/gcc
PYTHON_CONFIG = $(PYTHON)-config
PG_CONFIG ?= pg_config

EXTENSION = pgllm
ΕΧΤVERSION = 0.1.0
MODULE_big = $(EXTENSION)
OBJS = src/pgllm.o
HEADERS = src/pgllm.h

DATA = $(wildcard sql/*--*.sql)

TESTS = test/sql/setup.sql \
		test/sql/basic.sql \
		test/sql/gen_markov.sql \
		test/sql/embed_hazo.sql \
		test/sql/embed_onnx.sql \
		test/sql/embed_jina.sql

ifdef WITH_LLAMAFILE
TESTS += test/sql/gen_llamafile.sql
endif

REGRESS = $(patsubst test/sql/%.sql,%,$(TESTS))
REGRESS_OPTS = --inputdir=test --load-extension=$(EXTENSION)

PG_CFLAGS ?= $(shell $(PYTHON)-config --includes)
PG_LDFLAGS ?= $(shell $(PYTHON)-config --ldflags)
SHLIB_LINK += -lpython$(PYVERSION)

ifdef WITH_LLAMAFILE
PG_CFLAGS += -DWITH_LLAMAFILE
CURL_CONFIG = curl-config
CFLAGS += $(shell $(CURL_CONFIG) --cflags)
SHLIB_LINK += $(shell $(CURL_CONFIG) --libs)
endif

PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

dev: clean all install installcheck