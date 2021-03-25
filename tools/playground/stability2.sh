#!/bin/bash

BASE="https://sra-download.be-md.ncbi.nlm.nih.gov/sos3/vdb3testbucket/"

ACCESSIONS="SRR7392459 SRR7341916 SRR7158431 SRR8001010 SRR7796424 SRR7157007 SRR7389150 SRR7584907 SRR7981406 SRR8256711"

TODAY=`date +%m%d%H%M`

REPORT="report2_$TODAY.txt"
EREPORT="errors2_$TODAY.txt"
STOPFILE="stop"

rm -rf $REPORT $EREPORT

#
#   'touch stop' in other terminal terminates the script ( after a whole loop )
#

function fastq() {
    URL="$1"
    date >> $REPORT
    echo "$URL" >> $REPORT
    /usr/bin/time -f %E -o t.txt fastq-dump -+ KNS-HTTP -Z $URL > data.txt 2> data.err
    RET="$?"
    md5sum data.txt >> $REPORT
    cat t.txt >> $REPORT
    if [ $RET -ne 0 ]; then
        date >> $EREPORT
        echo "$URL" >> $EREPORT
        cat t.txt >> $EREPORT
        cat data.err >> $EREPORT
        echo "." >> $EREPORT
    fi
    rm data.err data.txt t.txt
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
