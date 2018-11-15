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

#-------------------------------------------------------------------------------
# targets
#
# help 		- list targets
# build		- build (default)
# config 	- display(/set?) configuration
# clean 	- remove build outputs
# test 		- run tests
# install 	- install on the development system (may require sudo)
# package 	- create a Docker container with a full installation
# docs 		- generate Doxygen

#-------------------------------------------------------------------------------
# default
#
default: build

#-------------------------------------------------------------------------------
# environment
#
TOP ?= $(CURDIR)
SUBDIRS =

include $(TOP)/build/Makefile.shell

#-------------------------------------------------------------------------------
# help
#
help:
	@echo "Supported targets:"
	@echo "    help     - list targets"
	@echo "    build    - build (default)"
	@echo "    config   - change/display configuration"
	@echo "    clean    - remove build outputs"
	@echo "    test     - run tests"
	@echo "    install  - install on the development system (may require sudo)"
	@echo "    package  - create a Docker container with a full installation"
	@echo "    docs     - generate Doxygen"

#-------------------------------------------------------------------------------
# config
#
config:
	@echo "TBD"

#-------------------------------------------------------------------------------
# clean
#
clean:
	@echo "TBD"

#-------------------------------------------------------------------------------
# build
#
build:
	@echo "TBD"

#-------------------------------------------------------------------------------
# test
#
test:
	make -C build/test

#-------------------------------------------------------------------------------
# install
#
install:
	@echo "TBD"

#-------------------------------------------------------------------------------
# package
#
package:
	@echo "TBD"

#-------------------------------------------------------------------------------
# docs
#
docs:
	@echo "TBD"


.PHONY: help config clean build test install package docs
