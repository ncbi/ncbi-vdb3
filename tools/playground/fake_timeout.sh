#!/bin/bash

BASE="https://sra-download.be-md.ncbi.nlm.nih.gov/sos3/vdb3testbucket/"
ACC="SRR7392459"
URL="$BASE$ACC.bits/$ACC"
ERROUT="fake_err.txt"
NORMOUT="fake_std.txt"
FAKE="NCBI_VDB_ERR_MBEDTLS_READ=1000"
RELIABLE="NCBI_VDB_RELIABLE=1"

export $FAKE
export $RELIABLE
fastq-dump $URL -+ KNS-HTTP -Z > $NORMOUT 2> $ERROUT
