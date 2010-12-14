SUBDIRS = .

AM_CPPFLAGS  = -I$(top_builddir)/src/lib -I$(top_srcdir)/src/lib
AM_CPPFLAGS += -I$(top_srcdir)/src/lib/dns -I$(top_builddir)/src/lib/dns
AM_CPPFLAGS += -I$(top_srcdir)/src/lib/nsas -I$(top_builddir)/src/lib/nsas
AM_CXXFLAGS = $(B10_CXXFLAGS)

AM_LDFLAGS =
if USE_STATIC_LINK
AM_LDFLAGS += -static
endif
AM_LDFLAGS += -lboost_thread

if USE_GXX
# Some versions of GCC warn about some versions of Boost regarding
# missing initializer for members in its posix_time.
# https://svn.boost.org/trac/boost/ticket/3477
AM_CXXFLAGS += -Wno-missing-field-initializers
endif

CLEANFILES = *.gcno *.gcda

TESTS =
if HAVE_GTEST
TESTS += run_unittests
run_unittests_SOURCES  = run_unittests.cc
run_unittests_SOURCES += address_entry_unittest.cc
run_unittests_SOURCES += hash_deleter_unittest.cc
run_unittests_SOURCES += hash_key_unittest.cc
run_unittests_SOURCES += hash_table_unittest.cc
run_unittests_SOURCES += hash_unittest.cc
run_unittests_SOURCES += lru_list_unittest.cc
run_unittests_SOURCES += nameserver_address_unittest.cc
run_unittests_SOURCES += nameserver_address_store_unittest.cc
run_unittests_SOURCES += nameserver_entry_unittest.cc
run_unittests_SOURCES += nsas_entry_compare_unittest.cc
run_unittests_SOURCES += nsas_test.h
run_unittests_SOURCES += zone_entry_unittest.cc
run_unittests_SOURCES += fetchable_unittest.cc
run_unittests_SOURCES += random_number_generator_unittest.cc

run_unittests_CPPFLAGS = $(AM_CPPFLAGS) $(GTEST_INCLUDES)
run_unittests_LDFLAGS = $(AM_LDFLAGS) $(GTEST_LDFLAGS)
run_unittests_LDADD = $(GTEST_LDADD)

run_unittests_LDADD += $(top_builddir)/src/lib/nsas/libnsas.la
run_unittests_LDADD += $(top_builddir)/src/lib/dns/libdns++.la
run_unittests_LDADD += $(top_builddir)/src/lib/exceptions/libexceptions.la
endif

noinst_PROGRAMS = $(TESTS)