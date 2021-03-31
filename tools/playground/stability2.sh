#!/bin/bash

BASE="https://sra-download.be-md.ncbi.nlm.nih.gov/sos3/vdb3testbucket/"

ACCESSIONS="SRR7392459 SRR7341916 SRR7158431 SRR8001010 SRR7796424 SRR7157007 SRR7389150 SRR7584907 SRR7981406 SRR8256711"

TODAY=`date +%m%d%H%M`

REPORT="report2_$TODAY.txt"
EREPORT="errors2_$TODAY.txt"
RECOVER="recover2_$TODAY.txt"
TIMING="t2.txt"
STOPFILE="stop"
TEMPSTDOUT="data2.txt"
TEMPSTDERR="err2.txt"

rm -rf $REPORT $EREPORT $RECOVER

#
#   'touch stop' in other terminal terminates the script ( after a whole loop )
#

function fastq() {
    URL="$1"
    date >> $REPORT
    echo "$URL" >> $REPORT
    /usr/bin/time -f %E -o $TIMING fastq-dump -+ KNS-HTTP -Z $URL > $TEMPSTDOUT 2> $TEMPSTDERR
    RET="$?"
    md5sum $TEMPSTDOUT >> $REPORT
    cat $TIMING >> $REPORT
    if [ $RET -ne 0 ]; then
        date >> $EREPORT
        echo "$URL" >> $EREPORT
        cat $TIMING >> $EREPORT
        cat $TEMPSTDERR >> $EREPORT
        echo "." >> $EREPORT
    else
        cat $TEMPSTDERR | grep "rcExhausted" > /dev/null
        GRET="$?"
        if [ $GRET -eq 0 ]; then
            date >> $RECOVER
            echo "$URL" >> $RECOVER
            cat $TIMING >> $RECOVER
            cat $TEMPSTDERR >> $RECOVER
            echo "." >> $RECOVER
        fi
    fi
    rm -f $TEMPSTDERR $TEMPSTDOUT $TIMING
    echo "return-code = $RET" >> $REPORT
    echo "." >> $REPORT
}

while true; do
    for acc in $ACCESSIONS; do
        if [ ! -f "$STOPFILE" ]; then
            echo "running $acc"
            fastq "$BASE$acc.bits/$acc"
        fi
    done

    sleep 20
    if [ -f "$STOPFILE" ]; then
        rm "$STOPFILE"
        echo "we are done"
        exit 0
    fi
done
