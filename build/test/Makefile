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
default: run

MAKE = make --no-print-directory
WORKDIR = .
ACTUAL = $(WORKDIR)/actual

LIBPLATFORM = libncbi-vdb3-platform.a

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

run:
	@ mkdir -p $(ACTUAL)
	@ cp Makefile.config.test-default Makefile.config
	@ echo "Testing build system"
	@ # 1. bad target
	@ $(call ExpectFail,1,-f ../../Makefile h)
	@ # 2. help
	@ $(call ExpectPass,2,-f ../../Makefile help)
	@ #
	@ # 3: platform/ sub-project
	@ #
	@ # 3.1 help
	@ $(call ExpectPass,3.1,-C platform/hello help)
	@ #
	@ # 3.2 build
	@ $(call Setup); \
	  $(call ExpectPass,3.2,-C platform/hello, \
	  						test -f $(ACTUAL)/outdir/dbg/obj/platform/hello/hello.pic.o)
	@ # 3.3 clean
	@ $(call Setup); \
	  $(call ExpectPass,3.3,-C platform/hello build clean, \
	  						! test -f $(ACTUAL)/outdir/dbg/obj/platform/hello/hello.pic.o)
	@ # 3.4 run all unit tests
	@ $(call Setup); \
	  $(call ExpectPass,3.4,-C platform/hello test, \
	  						[ 2 -eq $$(grep --count Passed $(ACTUAL)/3.4.stdout) ] )
	@ # 3.5  run a single unit test
	@ $(call Setup); \
	  $(call ExpectPass,3.5,-C platform/hello/test hello1, \
	  						[ 1 -eq $$(grep --count Passed $(ACTUAL)/3.5.stdout) ] )
	@ # 4. the platform library
	@ #
	@ # 4.1 build; $(LIBPLATFORM) created
	@ $(call Setup);\
	  $(call ExpectPass,4.1,-C platform build,\
	  						test -f $(ACTUAL)/outdir/dbg/lib/$(LIBPLATFORM))
	@ # 4.2 clean;
	@ $(call Setup); \
	  $(call ExpectPass,4.2,-C platform build clean,\
							! test -f $(ACTUAL)/outdir/dbg/lib/$(LIBPLATFORM))
	@ #
	@ # 4.3 test; expect 3 unit tests to be run
	@ $(call Setup); \
	  $(call ExpectPass,4.3,-C platform test, \
	  						[ 3 -eq $$(grep --count Passed $(ACTUAL)/4.3.stdout) ] )
	@ #
	@ #TODO: versioned binaries
	@ #TODO: config
	@ #TODO: default
	@ #TODO: test
	@ #TODO: install
	@ #TODO: package
	@ #TODO: docs
	@ #TODO: default (build) from a subdirectory
	@ #TODO: test from a subdirectory
	@ #TODO: default (test) from test/
	@ #TODO: clean from a subdirectory
	@ #TODO: config (read only) from a subdirectory
	@ #TODO: Release build

.phony: default run

