#!/bin/bash

BASE="https://sra-download.be-md.ncbi.nlm.nih.gov/sos3/vdb3testbucket/"

ACCESSIONS="SRR7392459 SRR7341916 SRR7158431 SRR8001010 SRR7796424 SRR7157007 SRR7389150 SRR7584907 SRR7981406 SRR8256711"


SDL="https://www.ncbi.nlm.nih.gov/Traces/sdl/2/retrieve"

TODAY=`date +%m%d%H%M`

REPORT="report1_$TODAY.txt"
EREPORT="errors1_$TODAY.txt"
TIMING="t1.txt"
STOPFILE="stop"

rm -rf $REPORT $EREPORT $TIMING

#
#   'touch stop' in other terminal terminates the script ( after a whole loop )
#

function get_url() {
    ACC="$1"
    SDLURL="$SDL?acc=$ACC&location=sra-ncbi&location-type=forced"
    result=`curl -s $SDLURL | jq -r '.result[0].files[0].locations[0].link'`
}

function fastq() {
    URL="$1"
    date >> $REPORT
    echo "$URL" >> $REPORT
    /usr/bin/time -f %E -o $TIMING fastq-dump -+ KNS-HTTP -Z $URL > data.txt 2> data.err
    RET="$?"
    md5sum data.txt >> $REPORT
    cat $TIMING >> $REPORT
    if [ $RET -ne 0 ]; then
        date >> $EREPORT
        echo "$URL" >> $EREPORT
        cat $TIMING >> $EREPORT
        cat data.err >> $EREPORT
        echo "." >> $EREPORT
    fi
    rm data.err data.txt $TIMING
    echo "return-code = $RET" >> $REPORT
    echo "." >> $REPORT
}

function loop1 {
for acc in $ACCESSIONS; do
    if [ ! -f "$STOPFILE" ]; then
        echo "running $acc"
        fastq "$BASE$acc.bits/$acc"
    fi
done
}

function loop2 {
for acc in $ACCESSIONS; do
    if [ ! -f "$STOPFILE" ]; then
        echo "running $acc"
        get_url $acc
        fastq "$result"
    fi
done
}

while true; do
    loop2
    sleep 20
    if [ -f "$STOPFILE" ]; then
        rm "$STOPFILE"
        echo "we are done"
        exit 0
    fi
done
