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

  fastq_vdb3_par)
    echo "fastq_vdb3_par selected"
    ;;

  *)
    echo "invalid selections"
    exit 3
    ;;
esac

DATABASE="stab2.db"
SPECIAL_BASE="https://sra-download.be-md.ncbi.nlm.nih.gov/sos3/vdb3testbucket/"
SDL="https://www.ncbi.nlm.nih.gov/Traces/sdl/2/retrieve"
ACCESSIONS="SRR7392459 SRR7341916 SRR7158431 SRR8001010 SRR7796424 SRR7157007 SRR7389150 SRR7584907 SRR7981406 SRR8256711"
TIMING="t_$SELECTION.txt"
TEMPSTDOUT="data_$SELECTION.txt"
TEMPSTDERR="err_$SELECTION.txt"
STOPFILE="stop_$SELECTION"
RELIABLE="NCBI_VDB_RELIABLE=1"
TIME_LIMIT="100"

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
            'runtime' REAL,
            'ret_code' INTEGER,
            'errors' TEXT DEFAULT '-' ) "
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
function insert_event1 {
    #give error-file a different name, to protect it from beeing overwritten in the next loop
    #store this filename in the errors-columns
    EDATE=`date "+%y%m%d_%H_%M_%S"`
    ENAME="${EDATE}_$1_$2_$3.errors"
    mv $TEMPSTDERR "${ENAME}"
    STM="INSERT INTO log ( reader, source, acc, url, md5, runtime, ret_code, errors ) VALUES ( '$1', '$2', '$3', '$4', '$5', '$6', '$7', '${ENAME}' )"
    sqlite3 $DATABASE "$STM"
}

#   no errors....
function insert_event2 {
    STM="INSERT INTO log ( reader, source, acc, url, md5, runtime, ret_code ) VALUES ( '$1', '$2', '$3', '$4', '$5', '$6', '$7' )"
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

#   $1 ... reader
#   $2 ... source
#   $3 ... acc
#   $4 ... url
#   $5 ... md5
#   $6 ... runtime
#   $7 ... ret-code
function record_results() {
    RR_READER=$1
    RR_SOURCE=$2
    RR_ACC=$3
    RR_URL=$4
    RR_MD5=$5
    RR_RT=$6
    RR_RET=$7
    if [ $RR_RET -ne 0 ]; then
        echo "$RR_READER $RR_SOURCE $ACC failed"
        insert_event1 "$RR_READER" "$RR_SOURCE" "$RR_ACC" "$RR_URL" "$RR_MD5" "$RR_RT" "$RR_RET"
    else
        echo "$RR_READER $RR_SOURCE $ACC succeeded"
        RT_SEC=`printf %.0f $RR_RT`
        if [ $RT_SEC -gt $TIME_LIMIT ]; then
            insert_event1 "$RR_READER" "$RR_SOURCE" "$RR_ACC" "$RR_URL" "$RR_MD5" "$RR_RT" "$RR_RET"
        else
            insert_event2 "$RR_READER" "$RR_SOURCE" "$RR_ACC" "$RR_URL" "$RR_MD5" "$RR_RT" "$RR_RET"
        fi
    fi
    rm -f $TEMPSTDOUT $TEMPSTDERR $TIMING
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
        record_results 'fastq' 'cloudian' "$ACC" "$URL" "$MD5" "$RT" "$RET"
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
    record_results 'fastq_spec' 'cloudian' "$ACC" "$URL" "$MD5" "$RT" "$RET"
}

function fastq_vdb3() {
    ACC="$1"
    URL="$SPECIAL_BASE$ACC.bits/"
    echo "$ACC at $URL"
    /usr/bin/time -f %e -o $TIMING ./reader2.py -U $URL > $TEMPSTDOUT 2> $TEMPSTDERR
    RET="$?"
    MD5=`cat $TEMPSTDOUT | md5sum | awk '{ print $1 }'`
    RT=`cat $TIMING`
    record_results 'vdb3' 'cloudian' "$ACC" "$URL" "$MD5" "$RT" "$RET"
}

function fastq_vdb3_par() {
    ACC="$1"
    URL="$SPECIAL_BASE$ACC.bits2/"
    echo "$ACC at $URL"
    /usr/bin/time -f %e -o $TIMING ./reader2.py -U $URL -p > $TEMPSTDOUT 2> $TEMPSTDERR
    RET="$?"
    MD5=`cat $TEMPSTDOUT | md5sum | awk '{ print $1 }'`
    RT=`cat $TIMING`
    record_results 'vdb3-par' 'cloudian' "$ACC" "$URL" "$MD5" "$RT" "$RET"
}

function fastq_sdl() {
    ACC="$1"
    URL=`srapath $ACC`
    echo "$ACC at $URL"
    /usr/bin/time -f %e -o $TIMING fastq-dump -+ KNS-HTTP -Z $ACC > $TEMPSTDOUT 2> $TEMPSTDERR
    RET="$?"
    MD5=`cat $TEMPSTDOUT | md5sum | awk '{ print $1 }'`
    RT=`cat $TIMING`
    record_results 'fastq' 'sdl' "$ACC" "$URL" "$MD5" "$RT" "$RET"
}

function exit_on_stopfile {
    if [ -f "$STOPFILE" ]; then
        rm "$STOPFILE"
        echo "we are done"
        exit 0
    fi
}

rm -f $TEMPSTDOUT $TEMPSTDERR
create_db

#make sure that computed URL's are treated the same as SDL-received ones
export $RELIABLE

while true; do
    for acc in $ACCESSIONS; do
        exit_on_stopfile
        case $SELECTION in
            fastq)
                fastq "$acc"
                ;;

            fastq_special)
                fastq_special "$acc"
                ;;

            fastq_vdb3)
                fastq_vdb3 "$acc"
                ;;

            fastq_vdb3_par)
                fastq_vdb3_par "$acc"
                ;;

            fastq_sdl)
                fastq_sdl "$acc"
                ;;

            *)
                echo "invalid selections"
                exit 1
                ;;
        esac
    done
    sleep 20
done
