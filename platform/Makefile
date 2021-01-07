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
MODULE = platform
SUBDIRS ?= memory secure json ncbi

include $(TOP)/$(MODULE)/Makefile.env

LIBRARY = $(LIBDIR)/libncbi-vdb3-platform.a
LIBRARY_NO_ATOMIC_OPS = $(LIBDIR)/libncbi-vdb3-platform-no-atomic-ops.a
LIBRARY_NO_BUSY_LOCK = $(LIBDIR)/libncbi-vdb3-platform-no-busy-lock.a

SUBDIRS_OBJ = $(addprefix $(OBJDIR)/, $(addsuffix /*.$(LOBX),$(SUBDIRS) ) )
SUBDIRS_NO_ATOMIC_OPS_OBJ = $(subst .$(LOBX),.no-atomic-ops-$(LOBX),$(SUBDIRS_OBJ))
SUBDIRS_NO_BUSY_LOCK_OBJ = $(subst .$(LOBX),.no-busy-lock-$(LOBX),$(SUBDIRS_OBJ))

build: makedirs
	$(AR) $(LIBRARY) $(SUBDIRS_OBJ)
ifeq (rel,$(BUILD))
	$(AR) $(LIBRARY_NO_ATOMIC_OPS) $(SUBDIRS_NO_ATOMIC_OPS_OBJ)
	$(AR) $(LIBRARY_NO_BUSY_LOCK) $(SUBDIRS_NO_BUSY_LOCK_OBJ)
endif

clean:
	@ rm -rf $(LIBRARY)

