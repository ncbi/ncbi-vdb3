#!/bin/bash

BASE="https://sra-download.be-md.ncbi.nlm.nih.gov/sos3/vdb3testbucket/"

ACCESSIONS="SRR7392459 SRR7341916 SRR7158431 SRR8001010 SRR7796424 SRR7157007 SRR7389150 SRR7584907 SRR7981406 SRR8256711"

TODAY=`date +%m%d%H%M`

REPORT="report3_$TODAY.txt"
EREPORT="errors3_$TODAY.txt"
TIMING="t3.txt"
STOPFILE="stop"
TEMPSTDOUT="data3.txt"
TEMPSTDERR="err3.txt"

rm -rf $REPORT $EREPORT

#
#   'touch stop' in other terminal terminates the script ( after a whole loop )
#

function fastq() {
    URL="$1"
    date >> $REPORT
    echo "$URL" >> $REPORT
    /usr/bin/time -f %E -o $TIMING ./reader2.py -U $URL > $TEMPSTDOUT 2> $TEMPSTDERR
    RET="$?"
    md5sum $TEMPSTDOUT >> $REPORT
    cat $TIMING >> $REPORT
    if [ $RET -ne 0 ]; then
        date >> $EREPORT
        echo "$URL" >> $EREPORT
        cat $TIMING >> $EREPORT
        cat $TEMPSTDERR >> $EREPORT
        echo "." >> $EREPORT
    fi
    rm -f $TEMPSTDERR $TEMPSTDOUT $TIMING
    echo "return-code = $RET" >> $REPORT
    echo "." >> $REPORT
}

while true; do
    for acc in $ACCESSIONS; do
        if [ ! -f "$STOPFILE" ]; then
            echo "running $acc"
            fastq "$BASE$acc.bits"
        fi
    done

    sleep 20
    if [ -f "$STOPFILE" ]; then
        rm "$STOPFILE"
        echo "we are done"
        exit 0
    fi
done
