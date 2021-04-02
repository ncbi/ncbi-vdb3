#!/bin/bash

SELECTION=$1
case $SELECTION in
  fastq)
    echo "fastq selected"
    ;;

  fastq_special)
    echo "fastq_special selected"
    ;;

  fastq_vdb3)
    echo "fastq_vdb3 selected"
    ;;

  fastq_sdl)
    echo "fastq_sdl selected"
    ;;

  *)
    echo "invalid selections"
    exit 3
    ;;
esac

DATABASE="stab.db"
SPECIAL_BASE="https://sra-download.be-md.ncbi.nlm.nih.gov/sos3/vdb3testbucket/"
SDL="https://www.ncbi.nlm.nih.gov/Traces/sdl/2/retrieve"
ACCESSIONS="SRR7392459 SRR7341916 SRR7158431 SRR8001010 SRR7796424 SRR7157007 SRR7389150 SRR7584907 SRR7981406 SRR8256711"
TIMING="t_$SELECTION.txt"
TEMPSTDOUT="data_$SELECTION.txt"
TEMPSTDERR="err_$SELECTION.txt"
STOPFILE="stop_$SELECTION"
RELIABLE="NCBI_VDB_RELIABLE=1"

function create_db {
    STM="CREATE TABLE IF NOT EXISTS
        'log' ( 
            'id' INTEGER PRIMARY KEY,
            'ts' DATETIME NOT NULL DEFAULT ( datetime( 'now', 'localtime' ) ) ,
            'reader' TEXT,
            'source' TEXT,
            'acc' TEXT,
            'url' TEXT,
            'md5' TEXT,
            'runtime' TEXT,
            'ret_code' REAL,
            'errors' TEXT ) "
    sqlite3 $DATABASE "$STM"
    STM="CREATE VIEW IF NOT EXISTS
        'logview' AS SELECT id,ts,reader,source,acc,runtime,ret_code FROM log"
    sqlite3 $DATABASE "$STM"
}

#   $1 ... reader
#   $2 ... source
#   $3 ... acc
#   $4 ... url
#   $5 ... md5
#   $6 ... runtime
#   $7 ... ret-code
#   $8 ... filename of errors
function insert_event1 {
    STM="INSERT INTO log ( reader, source, acc, url, md5, runtime, ret_code, errors ) VALUES ( '$1', '$2', '$3', '$4', '$5', '$6', '$7', readfile( '$8' ) )"
    sqlite3 $DATABASE "$STM"
}

#   $8 ... errors as string
function insert_event2 {
    STM="INSERT INTO log ( reader, source, acc, url, md5, runtime, ret_code, errors ) VALUES ( '$1', '$2', '$3', '$4', '$5', '$6', '$7', '$8' )"
    sqlite3 $DATABASE "$STM"
}

function get_url() {
    ACC="$1"
    SDLURL="$SDL?acc=$ACC&location=sra-ncbi&location-type=forced"
    CURLRESPONSE=`curl -s $SDLURL`
    result=`echo $CURLRESPONSE | jq -r '.result[0].files[0].locations[0].link'`
    if [ "$result" == "null" ]; then
        echo "get_url( $ACC ) failed"
        insert_event2 'sdl' 'ncbi' "$ACC" "$SDLURL" "-" "-" "-" "$CURLRESPONSE"
    fi
}

function fastq() {
    ACC="$1"
    get_url $ACC
    if [ ! "$result" == "null" ]; then
        URL="$result"
        echo "$ACC at $URL"
        /usr/bin/time -f %e -o $TIMING fastq-dump -+ KNS-HTTP -Z $URL > $TEMPSTDOUT 2> $TEMPSTDERR
        RET="$?"
        MD5=`cat $TEMPSTDOUT | md5sum | awk '{ print $1 }'`
        RT=`cat $TIMING`
        if [ $RET -ne 0 ]; then
            echo "fastq-dump $ACC failed"
            insert_event1 'fastq' 'cloudian' "$ACC" "$URL" "$MD5" "$RT" "$RET" "$TEMPSTDERR"
        else
            echo "fastq-dump $ACC success"
            insert_event2 'fastq' 'cloudian' "$ACC" "$URL" "$MD5" "$RT" "$RET" "-"
        fi
        rm -f $TEMPSTDOUT $TEMPSTDERR $TIMING
    fi
}

function fastq_special() {
    ACC="$1"
    URL="$SPECIAL_BASE$ACC.bits/$ACC"
    echo "$ACC at $URL"
    /usr/bin/time -f %e -o $TIMING fastq-dump -+ KNS-HTTP -Z $URL > $TEMPSTDOUT 2> $TEMPSTDERR
    RET="$?"
    MD5=`cat $TEMPSTDOUT | md5sum | awk '{ print $1 }'`
    RT=`cat $TIMING`
    if [ $RET -ne 0 ]; then
        echo "fastq-dump $ACC failed"
        insert_event1 'fastq_spec' 'cloudian' "$ACC" "$URL" "$MD5" "$RT" "$RET" "$TEMPSTDERR"
    else
        echo "fastq-dump $ACC success"
        insert_event2 'fastq_spec' 'cloudian' "$ACC" "$URL" "$MD5" "$RT" "$RET" "-"
    fi
    rm -f $TEMPSTDOUT $TEMPSTDERR $TIMING
}

function fastq_vdb3() {
    ACC="$1"
    URL="$SPECIAL_BASE$ACC.bits/"
    echo "$ACC at $URL"
    /usr/bin/time -f %e -o $TIMING ./reader2.py -U $URL > $TEMPSTDOUT 2> $TEMPSTDERR
    RET="$?"
    MD5=`cat $TEMPSTDOUT | md5sum | awk '{ print $1 }'`
    RT=`cat $TIMING`
    if [ $RET -ne 0 ]; then
        echo "reader2.py $ACC failed"
        insert_event1 'vdb3' 'cloudian' "$ACC" "$URL" "$MD5" "$RT" "$RET" "$TEMPSTDERR"
    else
        echo "reader2.py $ACC success"
        insert_event2 'vdb3' 'cloudian' "$ACC" "$URL" "$MD5" "$RT" "$RET" "-"
    fi
    rm -f $TEMPSTDOUT $TEMPSTDERR $TIMING
}

function fastq_sdl() {
    ACC="$1"
    URL=`srapath $ACC`
    echo "$ACC at $URL"
    /usr/bin/time -f %e -o $TIMING fastq-dump -+ KNS-HTTP -Z $ACC > $TEMPSTDOUT 2> $TEMPSTDERR
    RET="$?"
    MD5=`cat $TEMPSTDOUT | md5sum | awk '{ print $1 }'`
    RT=`cat $TIMING`
    if [ $RET -ne 0 ]; then
        echo "fastq-dump $ACC failed"
        insert_event1 'fastq' 'sdl' "$ACC" "$URL" "$MD5" "$RT" "$RET" "$TEMPSTDERR"
    else
        echo "fastq-dump $ACC success"
        insert_event2 'fastq' 'sdl' "$ACC" "$URL" "$MD5" "$RT" "$RET" "-"
    fi
    rm -f $TEMPSTDOUT $TEMPSTDERR $TIMING
}

function exit_on_stopfile {
    if [ -f "$STOPFILE" ]; then
        rm "$STOPFILE"
        echo "we are done"
        exit 0
    fi
}

function fastq_loop {
    for acc in $ACCESSIONS; do
        exit_on_stopfile
        fastq "$acc"
    done
}

function fastq_special_loop {
    for acc in $ACCESSIONS; do
        exit_on_stopfile
        fastq_special "$acc"
    done
}

function fastq_vdb3_loop {
    for acc in $ACCESSIONS; do
        exit_on_stopfile
        fastq_vdb3 "$acc"
    done
}

function fastq_sdl_loop {
    for acc in $ACCESSIONS; do
        exit_on_stopfile
        fastq_sdl "$acc"
    done
}

function endless_loop_fastq {
    while true; do
        fastq_loop
        sleep 20
    done
}

function endless_loop_fastq_special {
    while true; do
        fastq_special_loop
        sleep 20
    done
}

function endless_loop_fastq_vdb3 {
    while true; do
        fastq_vdb3_loop
        sleep 20
    done
}

function endless_loop_fastq_sdl {
    while true; do
        fastq_sdl_loop
        sleep 20
    done
}

rm -f $TEMPSTDOUT $TEMPSTDERR
create_db

#make sure that computed URL's are treated the same as SDL-received ones
export $RELIABLE

case $SELECTION in
  fastq)
    endless_loop_fastq
    ;;

  fastq_special)
    endless_loop_fastq_special
    ;;

  fastq_vdb3)
    endless_loop_fastq_vdb3
    ;;

  fastq_sdl)
    endless_loop_fastq_sdl
    ;;

  *)
    echo "invalid selections"
    ;;
esac
