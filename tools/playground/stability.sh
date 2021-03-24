#!/bin/bash

BASE="https://sra-download.be-md.ncbi.nlm.nih.gov/sos3/vdb3testbucket/"

ACCESSIONS1="SRR13539474 SRR13541232 SRR13542424 SRR13543936 SRR13545812 SRR13549283 SRR13550272 SRR13551189 SRR13552622 SRR13635674"
ACCESSIONS2="SRR7392459 SRR7341916 SRR7158431 SRR8001010 SRR7796424 SRR7157007 SRR7389150 SRR7584907 SRR7981406 SRR8256711"


SDL="https://www.ncbi.nlm.nih.gov/Traces/sdl/2/retrieve"

REPORT="report2.txt"
EREPORT="errors2.txt"

rm -rf $REPORT $EREPORT

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

function loop1 {
for acc in $ACCESSIONS1; do
    echo "running $acc"
    fastq "$BASE$acc.bits/$acc"
done
}

function loop2 {
for acc in $ACCESSIONS2; do
    echo "running $acc"
    get_url $acc
    fastq "$result"
done
}

loop2
exit 0

while true; do
    one_loop
    sleep 20
    if [ -f "stop" ]; then
        rm stop
        echo "we are done"
        exit 0
    fi
done
