#!/bin/bash

SELECTION=$1
case $SELECTION in
   not_partial)
    echo "not_partial"
    ;;

  partial_cloudian_seq)
    echo "partial_cloudian_seq selected"
    ;;

  partial_cloudian_rand)
    echo "partial_cloudian_rand selected"
    ;;

  partial_amazon_seq)
    echo "partial_amazon_seq selected"
    ;;

  partial_amazon_rand)
    echo "partial_amazon_rand selected"
    ;;

  *)
    echo "invalid selections"
    exit 3
    ;;
esac

DATABASE="timing.db"
TIMING="t_$SELECTION.txt"
STOPFILE="stop_$SELECTION"

function create_db {
    STM="CREATE TABLE IF NOT EXISTS
        'log' ( 
            'id' INTEGER PRIMARY KEY,
            'ts' DATETIME NOT NULL DEFAULT ( datetime( 'now', 'localtime' ) ) ,
            'reader' TEXT,
            'runtime' REAL,
            'ret_code' INTEGER ) "
    sqlite3 $DATABASE "$STM"
}

#   $1 ... reader
#   $2 ... runtime
#   $3 ... ret-code
function insert_event {
    STM="INSERT INTO log ( reader, runtime, ret_code ) VALUES ( '$1', '$2', '$3' )"
    sqlite3 $DATABASE "$STM"
}

#   $1 ... reader
#   $2 ... runtime
#   $3 ... ret-code
function record_results() {
    RR_READER=$1
    RR_RT=$2
    RR_RET=$3
    if [ $RR_RET -ne 0 ]; then
        echo "$RR_READER failed"
    else
        echo "$RR_READER succeeded"
    fi
    insert_event "$RR_READER" "$RR_RT" "$RR_RET"    
    rm -f $TIMING
}

function not_partial() {
    /usr/bin/time -f %e -o $TIMING ./partial.py
    RET="$?"
    RT=`cat $TIMING`
    record_results 'not_partial' "$RT" "$RET"
}

function partial_cloudian_seq() {
    /usr/bin/time -f %e -o $TIMING ./partial.py --partial
    RET="$?"
    RT=`cat $TIMING`
    record_results 'partial_cloudian_seq' "$RT" "$RET"
}

function partial_cloudian_rand() {
    /usr/bin/time -f %e -o $TIMING ./partial.py --partial --random
    RET="$?"
    RT=`cat $TIMING`
    record_results 'partial_cloudian_rand' "$RT" "$RET"
}

function partial_amazon_seq() {
    /usr/bin/time -f %e -o $TIMING ./partial.py --partial --amazon
    RET="$?"
    RT=`cat $TIMING`
    record_results 'partial_amazon_seq' "$RT" "$RET"
}

function partial_amazon_rand() {
    /usr/bin/time -f %e -o $TIMING ./partial.py --partial --random --amazon
    RET="$?"
    RT=`cat $TIMING`
    record_results 'partial_amazon_rand' "$RT" "$RET"
}

function exit_on_stopfile {
    if [ -f "$STOPFILE" ]; then
        rm "$STOPFILE"
        echo "we are done"
        exit 0
    fi
}

create_db

while true; do
    exit_on_stopfile
    $SELECTION
    sleep 20
done
