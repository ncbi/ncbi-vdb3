# ===========================================================================
#
#                            PUBLIC DOMAIN NOTICE
#               National Center for Biotechnology Information
#
#  This software/database is a "United States Government Work" under the
#  terms of the United States Copyright Act.  It was written as part of
#  the author's official duties as a United States Government employee and
#  thus cannot be copyrighted.  This software/database is freely available
#  to the public for use. The National Library of Medicine and the U.S.
#  Government have not placed any restriction on its use or reproduction.
#
#  Although all reasonable efforts have been taken to ensure the accuracy
#  and reliability of the software and data, the NLM and the U.S.
#  Government do not and cannot warrant the performance or results that
#  may be obtained by using this software or data. The NLM and the U.S.
#  Government disclaim all warranties, express or implied, including
#  warranties of performance, merchantability or fitness for any particular
#  purpose.
#
#  Please cite the author in any work or product based on this material.
#
# ===========================================================================

#
#	Makefile for the platform subsystem of VDB3
#

#-------------------------------------------------------------------------------
# default
#
default: build

TOP ?= $(abspath ..)
MODULE = external
SUBDIRS ?=

include $(TOP)/$(MODULE)/Makefile.env

build: utf8proc mbedtls gtest

clean: utf8proc_clean mbedtls_clean gtest_clean

GTESTLIBS = \
	$(LIBDIR)/libgtest.a

gtest: $(GTESTLIBS)

$(OBJDIR)/gtest-all.$(OBJX): googletest/googletest/src/gtest-all.cc
	$(CP) -g -c $< -o $@ $(CFLAGS) -Igoogletest/googletest/include -Igoogletest/googletest -MD

$(LIBDIR)/libgtest.a: $(OBJDIR)/gtest-all.$(OBJX)
	$(AR) $(LIBDIR)/libgtest.a $^

gtest_clean:
	@ rm -rf $(GTESTLIBS)

## utf8proc
UTF8LIBS =                    \
	$(LIBDIR)/libutf8proc.a

utf8proc: $(UTF8LIBS)

$(LIBDIR)/libutf8proc.a: utf8proc/libutf8proc.a
	cp $< $@

utf8proc/libutf8proc.a:
	$(MAKE) -C utf8proc

utf8proc_clean:
	$(MAKE) -C utf8proc clean
	@ rm -rf $(UTF8LIBS)

## mbedtls
MBEDLIBS =                    \
	$(LIBDIR)/libmbedcrypto.a \
	$(LIBDIR)/libmbedx509.a   \
	$(LIBDIR)/libmbedtls.a

mbedtls: $(MBEDLIBS)

$(LIBDIR)/libmbedcrypto.a: mbedtls/libmbedcrypto.a
	cp $< $@

mbedtls/libmbedcrypto.a:
	$(MAKE) -C mbedtls DEBUG=1 CFLAGS=-I../inc libmbedcrypto.a

$(LIBDIR)/libmbedx509.a: mbedtls/libmbedx509.a
	cp $< $@

mbedtls/libmbedx509.a:
	$(MAKE) -C mbedtls CFLAGS=-I../inc libmbedx509.a

$(LIBDIR)/libmbedtls.a: mbedtls/libmbedtls.a
	cp $< $@

mbedtls/libmbedtls.a:
	$(MAKE) -C mbedtls CFLAGS=-I../inc libmbedtls.a

mbedtls_clean:
	@ rm -rf $(MBEDLIBS)
	$(MAKE) -C mbedtls clean


