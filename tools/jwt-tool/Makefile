default: all

build: all

TOP ?= $(abspath ../..)
MODULE = tools/jwt-tool
SUBDIRS ?=

include ../Makefile.env

ALL_LIBS =      \
	ncbi-json   \
	ncbi-jwa    \
	ncbi-jwk    \
	ncbi-jws    \
	ncbi-jwe    \
	ncbi-jwt    \

#	ncbi-token  \

ALL_TOOLS =     \
	jwt-tool

all: $(ALL_LIBS) $(ALL_TOOLS)

fuzz: ncbi-oauth-fuzz

TSTDIR ?= $(CURDIR)/tst

#BINDIR ?= $(CURDIR)/bin
#LIBDIR ?= $(CURDIR)/lib
#OBJDIR ?= $(CURDIR)/obj

$(TSTDIR) $(BINDIR) $(LIBDIR) $(OBJDIR):
	mkdir -p $@

# determine OS
OS = unknown
UNAME = $(shell uname -s)
ifeq (Darwin, $(UNAME))
	OS = mac
	CFLAGS += -DMAC -DBSD -DUNIX
	UUIDLIB =
	PTHREADLIB =
	LQUADMATH =
endif
ifeq (Linux, $(UNAME))
	OS = linux
	CFLAGS += -DLINUX -DUNIX
	UUIDLIB = -luuid
	PTHREADLIB = -lpthread
	LQUADMATH = -lquadmath
endif

# only for Linux and MacOS
OBJX = o
LOBX = pic.o

# tell make to forget some of its built-in rules
.SUFFIXES:

# C++ compiler to use
GPP ?= g++ -std=c++11

CFLAGS += -I $(TOP)/external/utf8proc
CFLAGS += -I $(TOP)/external/inc
CFLAGS += -I $(TOP)/external/googletest/googletest/include
CFLAGS += -I $(TOP)/interfaces

## rules to build object files from source
$(OBJDIR)/%.dbg.$(OBJX): json/%.cpp
	$(GPP) -g -c $< -o $@ $(CFLAGS) -D_DEBUGGING -Ijson -Iinc -MD -Wall
$(OBJDIR)/%.tst.dbg.$(OBJX): json/%.cpp
	$(GPP) -g -c $< -DJSON_TESTING -o $@ $(CFLAGS) -D_DEBUGGING -Ijson -Iinc -MD -Wall
$(OBJDIR)/%.dbg.$(LOBX): json/%.cpp
	$(GPP) -g -c $< -o $@ $(CFLAGS) -D_DEBUGGING -Ijson -Iinc -fPIC -MD -Wall
$(OBJDIR)/%.tst.dbg.$(LOBX): json/%.cpp
	$(GPP) -g -c $< -DJSON_TESTING -o $@ $(CFLAGS) -D_DEBUGGING -Ijson -Iinc -fPIC -MD -Wall

$(OBJDIR)/%.rel.$(OBJX): json/%.cpp
	$(GPP) -O3 -c $< -o $@ $(CFLAGS) -Ijson -Iinc -MD -Wall
$(OBJDIR)/%.tst.rel.$(OBJX): json/%.cpp
	$(GPP) -O3 -c $< -DJSON_TESTING -o $@ $(CFLAGS) -Ijson -Iinc -MD -Wall
$(OBJDIR)/%.rel.$(LOBX): json/%.cpp
	$(GPP) -O3 -c $< -o $@ $(CFLAGS) -Ijson -Iinc -fPIC -MD -Wall
$(OBJDIR)/%.tst.rel.$(LOBX): json/%.cpp
	$(GPP) -O3 -c $< -DJSON_TESTING -o $@ $(CFLAGS) -Ijson -Iinc -fPIC -MD -Wall

$(OBJDIR)/%.dbg.$(OBJX): jwa/%.cpp
	$(GPP) -g -c $< -o $@ $(CFLAGS) -D_DEBUGGING -Ijwa -Iinc -MD -Wall
$(OBJDIR)/%.tst.dbg.$(OBJX): jwa/%.cpp
	$(GPP) -g -c $< -DJWA_TESTING -o $@ $(CFLAGS) -D_DEBUGGING -Ijwa -Iinc -MD -Wall
$(OBJDIR)/%.dbg.$(LOBX): jwa/%.cpp
	$(GPP) -g -c $< -o $@ $(CFLAGS) -D_DEBUGGING -Ijwa -Iinc -fPIC -MD -Wall
$(OBJDIR)/%.tst.dbg.$(LOBX): jwa/%.cpp
	$(GPP) -g -c $< -DJWA_TESTING -o $@ $(CFLAGS) -D_DEBUGGING -Ijwa -Iinc -fPIC -MD -Wall

$(OBJDIR)/%.rel.$(OBJX): jwa/%.cpp
	$(GPP) -O3 -c $< -o $@ $(CFLAGS) -Ijwa -Iinc -MD -Wall
$(OBJDIR)/%.tst.rel.$(OBJX): jwa/%.cpp
	$(GPP) -O3 -c $< -DJWA_TESTING -o $@ $(CFLAGS) -Ijwa -Iinc -MD -Wall
$(OBJDIR)/%.rel.$(LOBX): jwa/%.cpp
	$(GPP) -O3 -c $< -o $@ $(CFLAGS) -Ijwa -Iinc -fPIC -MD -Wall
$(OBJDIR)/%.tst.rel.$(LOBX): jwa/%.cpp
	$(GPP) -O3 -c $< -DJWA_TESTING -o $@ $(CFLAGS) -Ijwa -Iinc -fPIC -MD -Wall

$(OBJDIR)/%.dbg.$(OBJX): jwk/%.cpp
	$(GPP) -g -c $< -o $@ $(CFLAGS) -D_DEBUGGING -Ijwk -Ijwa -Iinc -MD -Wall
$(OBJDIR)/%.tst.dbg.$(OBJX): jwk/%.cpp
	$(GPP) -g -c $< -DJWK_TESTING -o $@ $(CFLAGS) -D_DEBUGGING -Ijwk -Iinc -MD -Wall
$(OBJDIR)/%.dbg.$(LOBX): jwk/%.cpp
	$(GPP) -g -c $< -o $@ $(CFLAGS) -D_DEBUGGING -Ijwk -Iinc -fPIC -MD -Wall
$(OBJDIR)/%.tst.dbg.$(LOBX): jwk/%.cpp
	$(GPP) -g -c $< -DJWK_TESTING -o $@ $(CFLAGS) -D_DEBUGGING -Ijwk -Iinc -fPIC -MD -Wall

$(OBJDIR)/%.rel.$(OBJX): jwk/%.cpp
	$(GPP) -O3 -c $< -o $@ $(CFLAGS) -Ijwk -Iinc -MD -Wall
$(OBJDIR)/%.tst.rel.$(OBJX): jwk/%.cpp
	$(GPP) -O3 -c $< -DJWK_TESTING -o $@ $(CFLAGS) -Ijwk -Iinc -MD -Wall
$(OBJDIR)/%.rel.$(LOBX): jwk/%.cpp
	$(GPP) -O3 -c $< -o $@ $(CFLAGS) -Ijwk -Iinc -fPIC -MD -Wall
$(OBJDIR)/%.tst.rel.$(LOBX): jwk/%.cpp
	$(GPP) -O3 -c $< -DJWK_TESTING -o $@ $(CFLAGS) -Ijwk -Iinc -fPIC -MD -Wall

$(OBJDIR)/%.dbg.$(OBJX): jws/%.cpp
	$(GPP) -g -c $< -o $@ $(CFLAGS) -D_DEBUGGING -Ijws -Ijwa -Iinc -MD -Wall
$(OBJDIR)/%.tst.dbg.$(OBJX): jws/%.cpp
	$(GPP) -g -c $< -DJWS_TESTING -o $@ $(CFLAGS) -D_DEBUGGING -Ijws -Iinc -MD -Wall
$(OBJDIR)/%.dbg.$(LOBX): jws/%.cpp
	$(GPP) -g -c $< -o $@ $(CFLAGS) -D_DEBUGGING -Ijws -Iinc -fPIC -MD -Wall
$(OBJDIR)/%.tst.dbg.$(LOBX): jws/%.cpp
	$(GPP) -g -c $< -DJWS_TESTING -o $@ $(CFLAGS) -D_DEBUGGING -Ijws -Iinc -fPIC -MD -Wall

$(OBJDIR)/%.rel.$(OBJX): jws/%.cpp
	$(GPP) -O3 -c $< -o $@ $(CFLAGS) -Ijws -Iinc -MD -Wall
$(OBJDIR)/%.tst.rel.$(OBJX): jws/%.cpp
	$(GPP) -O3 -c $< -DJWS_TESTING -o $@ $(CFLAGS) -Ijws -Iinc -MD -Wall
$(OBJDIR)/%.rel.$(LOBX): jws/%.cpp
	$(GPP) -O3 -c $< -o $@ $(CFLAGS) -Ijws -Iinc -fPIC -MD -Wall
$(OBJDIR)/%.tst.rel.$(LOBX): jws/%.cpp
	$(GPP) -O3 -c $< -DJWS_TESTING -o $@ $(CFLAGS) -Ijws -Iinc -fPIC -MD -Wall

$(OBJDIR)/%.dbg.$(OBJX): jwe/%.cpp
	$(GPP) -g -c $< -o $@ $(CFLAGS) -D_DEBUGGING -Ijwe -Ijwa -Iinc -MD -Wall
$(OBJDIR)/%.tst.dbg.$(OBJX): jwe/%.cpp
	$(GPP) -g -c $< -DJWE_TESTING -o $@ $(CFLAGS) -D_DEBUGGING -Ijwe -Iinc -MD -Wall
$(OBJDIR)/%.dbg.$(LOBX): jwe/%.cpp
	$(GPP) -g -c $< -o $@ $(CFLAGS) -D_DEBUGGING -Ijwe -Iinc -fPIC -MD -Wall
$(OBJDIR)/%.tst.dbg.$(LOBX): jwe/%.cpp
	$(GPP) -g -c $< -DJWE_TESTING -o $@ $(CFLAGS) -D_DEBUGGING -Ijwe -Iinc -fPIC -MD -Wall

$(OBJDIR)/%.rel.$(OBJX): jwe/%.cpp
	$(GPP) -O3 -c $< -o $@ $(CFLAGS) -Ijwe -Iinc -MD -Wall
$(OBJDIR)/%.tst.rel.$(OBJX): jwe/%.cpp
	$(GPP) -O3 -c $< -DJWE_TESTING -o $@ $(CFLAGS) -Ijwe -Iinc -MD -Wall
$(OBJDIR)/%.rel.$(LOBX): jwe/%.cpp
	$(GPP) -O3 -c $< -o $@ $(CFLAGS) -Ijwe -Iinc -fPIC -MD -Wall
$(OBJDIR)/%.tst.rel.$(LOBX): jwe/%.cpp
	$(GPP) -O3 -c $< -DJWE_TESTING -o $@ $(CFLAGS) -Ijwe -Iinc -fPIC -MD -Wall

$(OBJDIR)/%.dbg.$(OBJX): jwt/%.cpp
	$(GPP) -g -c $< -o $@ $(CFLAGS) -D_DEBUGGING -Ijwt -Ijwa -Iinc -MD -Wall
$(OBJDIR)/%.tst.dbg.$(OBJX): jwt/%.cpp
	$(GPP) -g -c $< -DJWT_TESTING -o $@ $(CFLAGS) -D_DEBUGGING -Ijwt -Iinc -MD -Wall
$(OBJDIR)/%.dbg.$(LOBX): jwt/%.cpp
	$(GPP) -g -c $< -o $@ $(CFLAGS) -D_DEBUGGING -Ijwt -Iinc -fPIC -MD -Wall
$(OBJDIR)/%.tst.dbg.$(LOBX): jwt/%.cpp
	$(GPP) -g -c $< -DJWT_TESTING -o $@ $(CFLAGS) -D_DEBUGGING -Ijwt -Iinc -fPIC -MD -Wall

$(OBJDIR)/%.rel.$(OBJX): jwt/%.cpp
	$(GPP) -O3 -c $< -o $@ $(CFLAGS) -Ijwt -Iinc -MD -Wall
$(OBJDIR)/%.tst.rel.$(OBJX): jwt/%.cpp
	$(GPP) -O3 -c $< -DJWT_TESTING -o $@ $(CFLAGS) -Ijwt -Iinc -MD -Wall
$(OBJDIR)/%.rel.$(LOBX): jwt/%.cpp
	$(GPP) -O3 -c $< -o $@ $(CFLAGS) -Ijwt -Iinc -fPIC -MD -Wall
$(OBJDIR)/%.tst.rel.$(LOBX): jwt/%.cpp
	$(GPP) -O3 -c $< -DJWT_TESTING -o $@ $(CFLAGS) -Ijwt -Iinc -fPIC -MD -Wall

$(OBJDIR)/%.dbg.$(LOBX): token/%.cpp
	$(GPP) -g -c $< -o $@ $(CFLAGS) -D_DEBUGGING -Itoken -Iinc -fPIC -MD -Wall
$(OBJDIR)/%.tst.dbg.$(LOBX): token/%.cpp
	$(GPP) -g -c $< -DTOKEN_TESTING -o $@ $(CFLAGS) -D_DEBUGGING -Itoken -Iinc -fPIC -MD -Wall

$(OBJDIR)/%.dbg.$(OBJX): ncbi-oauth-test/%.cpp
	$(GPP) -g -c $< -o $@ $(CFLAGS) -D_DEBUGGING -Incbi-oauth-test -Iinc -MD -Wall

$(OBJDIR)/%.dbg.$(OBJX): tool/%.cpp
	$(GPP) -g -c $< -o $@ $(CFLAGS) -D_DEBUGGING -Itool -Iinc -MD -Wall
$(OBJDIR)/%.tst.dbg.$(OBJX): tool/%.cpp
	$(GPP) -g -c $< -DJWT_TOOL_TESTING -o $@ $(CFLAGS) -D_DEBUGGING -Itool -Iinc -MD -Wall

$(OBJDIR)/%.rel.$(OBJX): tool/%.cpp
	$(GPP) -O3 -c $< -o $@ $(CFLAGS) -Itool -Iinc -MD -Wall
$(OBJDIR)/%.tst.rel.$(OBJX): tool/%.cpp
	$(GPP) -O3 -c $< -DJWT_TOOL_TESTING -o $@ $(CFLAGS) -Itool -Iinc -MD -Wall

# include dependencies
include $(wildcard $(OBJDIR)/*.d)

## testing libs
TESTLIBS =       \
	-L$(LIBDIR)  \
	-lmbedcrypto \
	-lmbedx509   \
	-lmbedtls    \
	-lutf8proc   \
	-lgtest      \
	$(UUIDLIB)   \
	$(PTHREADLIB)\
	$(LQUADMATH) \

## ncbi-json
LIBJSONSRC =       \
	json           \
	json-object    \
	json-array     \
	json-value     \
	json-wrapper   \
	json-primitive

LIBJSONDBGOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .dbg.$(LOBX),$(LIBJSONSRC)))

LIBJSONRELOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .rel.$(LOBX),$(LIBJSONSRC)))

ncbi-json: $(LIBDIR) $(LIBDIR)/libncbi-json-dbg.a $(LIBDIR)/libncbi-json-rel.a

LIBJSONTESTS =                    \
	$(TSTDIR)/json-test.passed    \

$(LIBDIR)/libncbi-json-dbg.a: $(LIBJSONTESTS) $(OBJDIR) $(LIBJSONDBGOBJ) $(MAKEFILE)
	ar -rc $@ $(LIBJSONDBGOBJ)

$(LIBDIR)/libncbi-json-rel.a: $(LIBJSONTESTS) $(OBJDIR) $(LIBJSONRELOBJ) $(MAKEFILE)
	ar -rc $@ $(LIBJSONRELOBJ)

JSON_TSTSRC =  \
	json-tst

JSON_TSTOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .dbg.$(OBJX),$(JSON_TSTSRC)))

JSONTESTLIBS =        \
	-lncbi-vdb3-platform \
	-L$(LIBDIR)       \
	$(TESTLIBS)

$(TSTDIR)/json-test: $(TSTDIR) $(OBJDIR) $(JSON_TSTOBJ) $(MAKEFILE)
	$(GPP) -g -o $@ $(JSON_TSTOBJ) $(JSONTESTLIBS)

$(TSTDIR)/json-test.passed: $(TSTDIR)/json-test
	$< > $<.result && touch $@ || ( cat $<.result && false )

## ncbi-jwa
LIBJWASRC =        \
	jwa-rsa        \
	jwa-mgr        \
	jwa-none       \
	jwa-hmac       \
	jwa-ecdsa      \
	jwa-registry   \

LIBJWADBGOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .dbg.$(LOBX),$(LIBJWASRC)))

LIBJWARELOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .rel.$(LOBX),$(LIBJWASRC)))

ncbi-jwa: $(LIBDIR) $(LIBDIR)/libncbi-jwa-dbg.a $(LIBDIR)/libncbi-jwa-rel.a

LIBJWATESTS =                    \
	$(TSTDIR)/jwa-test.passed    \

$(LIBDIR)/libncbi-jwa-dbg.a: $(LIBJWATESTS) $(OBJDIR) $(LIBJWADBGOBJ) $(MAKEFILE)
	ar -rc $@ $(LIBJWADBGOBJ)

$(LIBDIR)/libncbi-jwa-rel.a: $(LIBJWATESTS) $(OBJDIR) $(LIBJWARELOBJ) $(MAKEFILE)
	ar -rc $@ $(LIBJWARELOBJ)

JWA_TSTSRC =        \
	jwa-tst         \
	jwk-key-cmn.tst

JWA_TSTOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .dbg.$(OBJX),$(JWA_TSTSRC)))

JWATESTDEPLIBS =                   \
	$(LIBDIR)/libncbi-json-dbg.a

JWATESTLIBS =         \
	-L$(LIBDIR)       \
	-lncbi-json-dbg   \
	-lncbi-vdb3-platform \
	$(TESTLIBS)

$(TSTDIR)/jwa-test: $(TSTDIR) $(OBJDIR) $(JWATESTDEPLIBS) $(JWA_TSTOBJ) $(MAKEFILE)
	$(GPP) -g -o $@ $(JWA_TSTOBJ) $(JWATESTLIBS)

$(TSTDIR)/jwa-test.passed: $(TSTDIR)/jwa-test
	$< > $<.result && touch $@ || ( cat $<.result && false )

## ncbi-jwk
LIBJWKSRC =        \
	jwk-mgr        \
	jwk-set        \
	jwk-key-cmn    \
	jwk-key

LIBJWKDBGOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .dbg.$(LOBX),$(LIBJWKSRC)))

LIBJWKRELOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .rel.$(LOBX),$(LIBJWKSRC)))

ncbi-jwk: $(LIBDIR) $(LIBDIR)/libncbi-jwk-dbg.a $(LIBDIR)/libncbi-jwk-rel.a

LIBJWKTESTS =                    \
	$(TSTDIR)/jwk-test.passed    \

$(LIBDIR)/libncbi-jwk-dbg.a: $(LIBJWKTESTS) $(OBJDIR) $(LIBJWKDBGOBJ) $(MAKEFILE)
	ar -rc $@ $(LIBJWKDBGOBJ)

$(LIBDIR)/libncbi-jwk-rel.a: $(LIBJWKTESTS) $(OBJDIR) $(LIBJWKRELOBJ) $(MAKEFILE)
	ar -rc $@ $(LIBJWKRELOBJ)

JWK_TSTSRC =  \
	jwk-tst

JWK_TSTOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .dbg.$(OBJX),$(JWK_TSTSRC)))

JWKTESTDEPLIBS =                   \
	$(LIBDIR)/libncbi-json-dbg.a

JWKTESTLIBS =         \
	-L$(LIBDIR)       \
	-lncbi-json-dbg   \
	-lncbi-vdb3-platform \
	$(TESTLIBS)

$(TSTDIR)/jwk-test: $(TSTDIR) $(OBJDIR) $(JWKTESTDEPLIBS) $(JWK_TSTOBJ) $(MAKEFILE)
	$(GPP) -g -o $@ $(JWK_TSTOBJ) $(JWKTESTLIBS)

$(TSTDIR)/jwk-test.passed: $(TSTDIR)/jwk-test
	$< > $<.result && touch $@ || ( cat $<.result && false )

## ncbi-jws
LIBJWSSRC =        \
	jws-mgr        \
	uri-mgr

LIBJWSDBGOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .dbg.$(LOBX),$(LIBJWSSRC)))

LIBJWSRELOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .rel.$(LOBX),$(LIBJWSSRC)))

ncbi-jws: $(LIBDIR) $(LIBDIR)/libncbi-jws-dbg.a $(LIBDIR)/libncbi-jws-rel.a

LIBJWSTESTS =                    \
	$(TSTDIR)/jws-test.passed    \

$(LIBDIR)/libncbi-jws-dbg.a: $(LIBJWSTESTS) $(OBJDIR) $(LIBJWSDBGOBJ) $(MAKEFILE)
	ar -rc $@ $(LIBJWSDBGOBJ)

$(LIBDIR)/libncbi-jws-rel.a: $(LIBJWSTESTS) $(OBJDIR) $(LIBJWSRELOBJ) $(MAKEFILE)
	ar -rc $@ $(LIBJWSRELOBJ)

JWS_TSTSRC =  \
	jws-tst

JWS_TSTOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .dbg.$(OBJX),$(JWS_TSTSRC)))

JWSTESTDEPLIBS =                   \
	$(LIBDIR)/libncbi-json-dbg.a   \
	$(LIBDIR)/libncbi-jwa-dbg.a    \
	$(LIBDIR)/libncbi-jwk-dbg.a

JWSTESTLIBS =         \
	-L$(LIBDIR)       \
	-lncbi-jwk-dbg    \
	-lncbi-jwa-dbg    \
	-lncbi-json-dbg   \
	-lncbi-vdb3-platform \
	$(TESTLIBS)

$(TSTDIR)/jws-test: $(TSTDIR) $(OBJDIR) $(JWSTESTDEPLIBS) $(JWS_TSTOBJ) $(MAKEFILE)
	$(GPP) -g -o $@ $(JWS_TSTOBJ) $(JWSTESTLIBS)

$(TSTDIR)/jws-test.passed: $(TSTDIR)/jws-test
	$< > $<.result && touch $@ || ( cat $<.result && false )

## ncbi-jwe
LIBJWESRC =        \
	jwe-mgr

LIBJWEDBGOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .dbg.$(LOBX),$(LIBJWESRC)))

LIBJWERELOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .rel.$(LOBX),$(LIBJWESRC)))

ncbi-jwe: $(LIBDIR) $(LIBDIR)/libncbi-jwe-dbg.a $(LIBDIR) $(LIBDIR)/libncbi-jwe-rel.a

LIBJWETESTS =                    \
	$(TSTDIR)/jwe-test.passed    \

$(LIBDIR)/libncbi-jwe-dbg.a: $(LIBJWETESTS) $(OBJDIR) $(LIBJWEDBGOBJ) $(MAKEFILE)
	ar -rc $@ $(LIBJWEDBGOBJ)

$(LIBDIR)/libncbi-jwe-rel.a: $(LIBJWETESTS) $(OBJDIR) $(LIBJWERELOBJ) $(MAKEFILE)
	ar -rc $@ $(LIBJWERELOBJ)

JWE_TSTSRC =  \
	jwe-tst

JWE_TSTOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .dbg.$(OBJX),$(JWE_TSTSRC)))

JWETESTDEPLIBS =                   \
	$(LIBDIR)/libncbi-json-dbg.a   \
	$(LIBDIR)/libncbi-jwa-dbg.a    \
	$(LIBDIR)/libncbi-jwk-dbg.a

JWETESTLIBS =         \
	-L$(LIBDIR)       \
	-lncbi-jwk-dbg    \
	-lncbi-jwa-dbg    \
	-lncbi-json-dbg   \
	-lncbi-vdb3-platform \
	$(TESTLIBS)

$(TSTDIR)/jwe-test: $(TSTDIR) $(OBJDIR) $(JWETESTDEPLIBS) $(JWE_TSTOBJ) $(MAKEFILE)
	$(GPP) -g -o $@ $(JWE_TSTOBJ) $(JWETESTLIBS)

$(TSTDIR)/jwe-test.passed: $(TSTDIR)/jwe-test
	$< > $<.result && touch $@ || ( cat $<.result && false )

## ncbi-jwt
LIBJWTSRC =            \
	jwt-mgr            \
	jwt-builder        \
	jwt-claims         \
	jwt-unverified

LIBJWTDBGOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .dbg.$(LOBX),$(LIBJWTSRC)))

LIBJWTRELOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .rel.$(LOBX),$(LIBJWTSRC)))

ncbi-jwt: $(LIBDIR) $(LIBDIR)/libncbi-jwt-dbg.a $(LIBDIR) $(LIBDIR)/libncbi-jwt-rel.a

LIBJWTTESTS =                    \
	$(TSTDIR)/jwt-test.passed    \

$(LIBDIR)/libncbi-jwt-dbg.a: $(LIBJWTTESTS) $(OBJDIR) $(LIBJWTDBGOBJ) $(MAKEFILE)
	ar -rc $@ $(LIBJWTDBGOBJ)

$(LIBDIR)/libncbi-jwt-rel.a: $(LIBJWTTESTS) $(OBJDIR) $(LIBJWTRELOBJ) $(MAKEFILE)
	ar -rc $@ $(LIBJWTRELOBJ)

JWT_TSTSRC =  \
	jwt-tst

JWT_TSTOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .dbg.$(OBJX),$(JWT_TSTSRC)))

JWTTESTDEPLIBS =                   \
	$(LIBDIR)/libncbi-json-dbg.a   \
	$(LIBDIR)/libncbi-jwa-dbg.a    \
	$(LIBDIR)/libncbi-jwk-dbg.a    \
	$(LIBDIR)/libncbi-jws-dbg.a    \
	$(LIBDIR)/libncbi-jwe-dbg.a

JWTTESTLIBS =         \
	-L$(LIBDIR)       \
	-lncbi-jwe-dbg    \
	-lncbi-jws-dbg    \
	-lncbi-jwk-dbg    \
	-lncbi-jwa-dbg    \
	-lncbi-json-dbg   \
	-lncbi-vdb3-platform \
	$(TESTLIBS)

$(TSTDIR)/jwt-test: $(TSTDIR) $(OBJDIR) $(JWTTESTDEPLIBS) $(JWT_TSTOBJ) $(MAKEFILE)
	$(GPP) -g -o $@ $(JWT_TSTOBJ) $(JWTTESTLIBS)

$(TSTDIR)/jwt-test.passed: $(TSTDIR)/jwt-test
	$< > $<.result && touch $@ || ( cat $<.result && false )

## ncbi-token
LIBTOKSRC =            \
	token-store        \
	token-set          \
	token-setentry

LIBTOKOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .dbg.$(LOBX),$(LIBTOKSRC)))

ncbi-token: $(LIBDIR) $(LIBDIR)/libncbi-token.a

$(LIBDIR)/libncbi-token.a: $(OBJDIR) $(LIBTOKOBJ) $(MAKEFILE)
	ar -rc $@ $(LIBTOKOBJ)

## jwt-tool
JWTTOOLSRC =            \
	jwt-tool            \
	init-jwt-tool       \
	exec-jwt-tool       \
	cleanup-jwt-tool    \
	env                 \
	cmdline             \
	log                 \
	log-local           \
	logger              \
	fmt

JWTTOOLDBGOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .dbg.$(OBJX),$(JWTTOOLSRC)))

JWTTOOLRELOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .rel.$(OBJX),$(JWTTOOLSRC)))

JWTTOOLDBGDEPLIBS =                \
	$(LIBDIR)/libncbi-json-dbg.a   \
	$(LIBDIR)/libncbi-jwa-dbg.a    \
	$(LIBDIR)/libncbi-jwk-dbg.a    \
	$(LIBDIR)/libncbi-jws-dbg.a    \
	$(LIBDIR)/libncbi-jwe-dbg.a    \
	$(LIBDIR)/libncbi-jwt-dbg.a

JWTTOOLDBGLIBS =      \
	-L$(LIBDIR)       \
	-lncbi-jwt-dbg    \
	-lncbi-jwe-dbg    \
	-lncbi-jws-dbg    \
	-lncbi-jwk-dbg    \
	-lncbi-jwa-dbg    \
	-lncbi-json-dbg   \
	-lncbi-vdb3-platform \
	$(TESTLIBS)

JWTTOOLRELDEPLIBS =                \
	$(LIBDIR)/libncbi-json-rel.a   \
	$(LIBDIR)/libncbi-jwa-rel.a    \
	$(LIBDIR)/libncbi-jwk-rel.a    \
	$(LIBDIR)/libncbi-jws-rel.a    \
	$(LIBDIR)/libncbi-jwe-rel.a    \
	$(LIBDIR)/libncbi-jwt-rel.a

JWTTOOLRELLIBS =      \
	-L$(LIBDIR)       \
	-lncbi-jwt-rel    \
	-lncbi-jwe-rel    \
	-lncbi-jws-rel    \
	-lncbi-jwk-rel    \
	-lncbi-jwa-rel    \
	-lncbi-json-rel   \
	-lncbi-vdb3-platform \
	$(TESTLIBS)

jwt-tool: $(BINDIR) $(TSTDIR)/jwt-tool-test.passed $(BINDIR)/jwt-tool-dbg $(BINDIR)/jwt-tool-rel

JWTTOOLTESTS =                        \
	$(TSTDIR)/jwt-tool-test.passed    \

$(BINDIR)/jwt-tool-dbg: $(OBJDIR) $(JWTTOOLDBGOBJ) $(JWTTOOLDBGDEPLIBS) $(MAKEFILE)
	$(GPP) -g -o $@ $(JWTTOOLDBGOBJ) $(JWTTOOLDBGLIBS)

$(BINDIR)/jwt-tool-rel: $(OBJDIR) $(JWTTOOLRELOBJ) $(JWTTOOLRELDEPLIBS) $(MAKEFILE)
	$(GPP) -O3 -o $@ $(JWTTOOLRELOBJ) $(JWTTOOLRELLIBS)

# JWT_TOOL_TSTSRC =

# JWT_TOOL_TSTOBJ = \
#  	$(addprefix $(OBJDIR)/,$(addsuffix .dbg.$(OBJX),$(JWT_TOOL_TSTSRC)))

# $(TSTDIR)/jwt-tool-test: $(TSTDIR) $(OBJDIR) $(JWTTESTDEPLIBS) $(JWT_TOOL_TSTOBJ) $(MAKEFILE)
# 	$(GPP) -g -o $@ $(JWT_TOOL_TSTOBJ) $(JWTTESTLIBS)

# $(TSTDIR)/jwt-tool-test.passed: $(TSTDIR)/jwt-tool-test
# 	$< > $<.result && touch $@ || ( cat $<.result && false )

$(TSTDIR)/jwt-tool-test.passed: $(BINDIR)/jwt-tool-dbg
	tool/tst-jwt-tool.sh $< > $<.result && touch $@ || ( cat $<.result && false )

## ncbi-oauth-fuzz
#
# requires clang 7
#

FUZZ_GPP = clang-7
FUZZ_FLAGS = -fsanitize=fuzzer,address,signed-integer-overflow
# with coverage/profile:
#FUZZ_FLAGS = -fsanitize=fuzzer,address,signed-integer-overflow -fprofile-instr-generate -fcoverage-mapping

# if there is a dictionary file, mention it here as -dict=$(path-to-file)
# FUZZ_DICT = -dict=fuzz-dict
FUZZ_DICT =

FUZZ_RUNS ?= -1

OAUTHFUZZSRC =    \
	fuzz-main

OAUTHFUZZOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .$(OBJX),$(OAUTHFUZZSRC))) \
	$(LIBJSONOBJ)

OAUTHFUZZLIB =   \
	-L$(LIBDIR)  \
	-lncbi-jwt   \
	-lmbedcrypto \
	-lmbedx509   \
	-lmbedtls    \
	-lpthread

has-clang:
	clang-7 -v >/dev/null 2>/dev/null || ( echo "fuzzing requires clang version 7 (clang-7) to be in the PATH" && exit 1 )

ncbi-oauth-fuzz: has-clang
	$(MAKE) OBJDIR=$(CURDIR)/obj/fuzz GPP=$(FUZZ_GPP) CFLAGS="$(FUZZ_FLAGS)" $(BINDIR)/ncbi-oauth-fuzz
	mkdir -p fuzz-corpus fuzz-seeds; $(BINDIR)/ncbi-oauth-fuzz -runs=$(FUZZ_RUNS) $(FUZZ_DICT) fuzz-corpus fuzz-seeds

$(BINDIR)/ncbi-oauth-fuzz: $(OBJDIR) $(OAUTHFUZZOBJ) $(MAKEFILE)
	$(GPP) $(CFLAGS) -g -o $@ $(OAUTHFUZZOBJ) $(OAUTHFUZZLIB)

clean:
	rm -rf $(OBJDIR) $(addprefix $(LIBDIR)/lib,$(addsuffix .a,$(ALL_LIBS)) ) $(BINDIR)/jwt-tool-dbg $(BINDIR)/jwt-tool-rel $(TSTDIR)

test: $(LIBSECTESTS) $(LIBJSONTESTS) $(LIBJWATESTS) $(LIBJWKTESTS) $(LIBJWSTESTS) $(LIBJWETESTS) $(LIBJWTTESTS) $(JWTTOOLTESTS)

.PHONY: default all ncbi-json ncbi-jwt ncbi-token jwt-tool mbedtls utf8proc

