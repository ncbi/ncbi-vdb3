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
#	Tests for the VDB3 build system
#

#-------------------------------------------------------------------------------
# default
#
default: test

TOP = $(abspath ../..)

MAKE = make --no-print-directory
WORKDIR = .
ACTUAL = $(WORKDIR)/actual

LIBPLATFORM = libncbi-vdb3-platform.a

build:

define Setup
    cp Makefile.config.test-default Makefile.config
	rm -rf $(ACTUAL)/outdir
endef

define ExpectPass
    ( $(MAKE) $(2) 1>$(ACTUAL)/$(1).stdout 2>$(ACTUAL)/$(1).stderr || \
		( echo "$(1) failed with $$? ($(MAKE) $(2))" && false ) ) && \
	( ( eval "$(3)" && echo "$(1) passed" ) || \
		echo "$(1) failed: $(3) " )
endef

define ExpectFail
    ! $(MAKE) $(2) 1>$(ACTUAL)/$(1).stdout 2>$(ACTUAL)/$(1).stderr && echo "$(1) passed"
endef

TEST_PLATFORM = -C platform SUBDIRS=hello\ goodbye -f $(TOP)/platform/Makefile

test:
	@ ln -f -s $(abspath ..) # in order to use the real files from $(TOP)/build
	@ mkdir -p $(ACTUAL)
	@ cp Makefile.config.test-default Makefile.config
	@ #
	@ echo "Testing build system"
	@ #
	@ # 1: platform/sub-project
	@ #
	@ # help
	@ $(call ExpectPass,1.1,-C platform/hello help)
	@ #
	@ # build
	@ $(call Setup); \
	  $(call ExpectPass,1.2,-C platform/hello build, \
	  						test -f $(ACTUAL)/outdir/dbg/obj/platform/hello/hello.pic.o)
	@ #
	@ # config
	@ $(call ExpectPass,1.3,-C platform/hello config)
	@ #
	@ # clean
	@ $(call Setup); \
	  $(call ExpectPass,1.4,-C platform/hello build clean, \
	  						! test -f $(ACTUAL)/outdir/dbg/obj/platform/hello/hello.pic.o)
	@ #
	@ # test (run all unit tests)
	@ $(call Setup); \
	  $(call ExpectPass,1.5,-C platform/hello test, \
	  						[ 2 -eq $$(grep --count Passed $(ACTUAL)/1.5.stdout) ] )
	@ #
	@ # 2: platform/sub-project/test
	@ #
	@ # help
	@ $(call ExpectPass,2.1,-C platform/hello/test help)
	@ #
	@ # build
	@ $(call Setup); \
	  $(call ExpectPass,2.2,-C platform/hello/test build, \
	  						test -f $(ACTUAL)/outdir/dbg/test-bin/test-hello1 )
	@ #
	@ # config
	@ $(call ExpectPass,2.3,-C platform/hello/test config)
	@ #
	@ # clean
	@ $(call Setup); \
	  $(call ExpectPass,2.4,-C platform/hello/test build clean, \
	  						! test -f $(ACTUAL)/outdir/dbg/test-bin/test-hello1 )
	@ #
	@ # run a single unit test
	@ $(call Setup); \
	  $(call ExpectPass,2.5,-C platform/hello/test hello1, \
	  						[ 1 -eq $$(grep --count Passed $(ACTUAL)/2.5.stdout) ] )
	@ #
	@ #
	@ # 5. the platform library
	@ #
	@ # help/
	@ $(call ExpectPass,3.1, $(TEST_PLATFORM) help)
	@ #
	@ # build; $(LIBPLATFORM) created
	@ $(call Setup);\
	  $(call ExpectPass,3.2, $(TEST_PLATFORM) build,\
	  						test -f $(ACTUAL)/outdir/dbg/lib/$(LIBPLATFORM))
	@ #
	@ # config
	@ $(call ExpectPass,3.3, $(TEST_PLATFORM) config)
	@ #
	@ # clean
	@ $(call Setup); \
	  $(call ExpectPass,3.4, $(TEST_PLATFORM) build clean,\
							! test -f $(ACTUAL)/outdir/dbg/lib/$(LIBPLATFORM))
	@ #
	@ # test; expect 3 unit tests to be run
	@ $(call Setup); \
	  $(call ExpectPass,3.5, $(TEST_PLATFORM) test, \
	  						[ 3 -eq $$(grep --count Passed $(ACTUAL)/3.5.stdout) ] )
	@ #
	@ #
	@ # 5. the db library
	@ #
	@ # help
	@ # $(call ExpectPass,5.1,-C db help)
	@ #
	@ #TODO: 5 services/
	@ #TODO: 6 tools/
	@ #TODO: 7 test/
	@ #
	@ # 8. Top directory
	@ #
	@ # bad target
	@ $(call ExpectFail,8.1,-f $(TOP)/Makefile h)
	@ #
	@ # help
	@ $(call ExpectPass,8.2,-f $(TOP)/Makefile help)
	@ #
	@ #TODO: build
	@ #TODO: config
	@ #		TODO: switch between Release/Debug
	@ #		TODO: switch between OUTDIRs
	@ #TODO: clean
	@ #TODO: test
	@ #TODO: install (?)
	@ #TODO: package
	@ #TODO: docs
	@ #
	@ #TODO: versioned binaries

