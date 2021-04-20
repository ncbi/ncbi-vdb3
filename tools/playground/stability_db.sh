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

  fastq_vdb3_aws)
    echo "fastq_vdb3_aws selected"
    ;;

  fastq_vdb3_par)
    echo "fastq_vdb3_par selected"
    ;;

  fastq_sdl)
    echo "fastq_sdl selected"
    ;;

  *)
    echo "invalid selections"
    exit 3
    ;;
esac

DATABASE="stab5.db"

SPECIAL_BASE="https://sra-download.be-md.ncbi.nlm.nih.gov/sos3/vdb3testbucket/"
AWS_BASE="https://vdb3.s3.amazonaws.com/"
SDL="https://www.ncbi.nlm.nih.gov/Traces/sdl/2/retrieve"

ACC1="SRR7392459 SRR7341916 SRR7158431 SRR8001010 SRR7796424"
ACC2="SRR7157007 SRR7389150 SRR7584907 SRR7981406 SRR8256711"

ACC3="SRR1155823 SRR1156826 SRR1157391 SRR1157964 SRR1157987"
ACC4="SRR1158069 SRR1158126 SRR1158196 SRR768882 SRR768866"

#ACCESSIONS="$ACC1 $ACC2 $ACC3 $ACC4"
#ACCESSIONS="$ACC1 $ACC2"
ACCESSIONS="$ACC3 $ACC4"

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
            'acc' TEXT,
            'runtime' REAL,
            'ret_code' INTEGER,
            'ressize' INTEGER,
            'pergb' REAL,
            'errors' TEXT DEFAULT '-' ) "
    sqlite3 $DATABASE "$STM"

    STM="CREATE TRIGGER IF NOT EXISTS tpermb AFTER INSERT ON log BEGIN
        UPDATE log SET pergb = ( runtime * 1024 * 1024 * 1024 ) / ressize WHERE id = NEW.id;
        END;"
    sqlite3 $DATABASE "$STM"
}

#   $1 ... reader
#   $2 ... acc
#   $3 ... runtime
#   $4 ... ret-code
#   $5 ... res-size
function insert_event1 {
    #give error-file a different name, to protect it from beeing overwritten in the next loop
    #store this filename in the errors-columns
    EDATE=`date "+%y%m%d_%H_%M_%S"`
    ENAME="${EDATE}_$1_$2.errors"
    mv $TEMPSTDERR "${ENAME}"
    STM="INSERT INTO log ( reader, acc, runtime, ret_code, ressize, errors ) VALUES ( '$1', '$2', '$3', '$4', '$5', '${ENAME}' )"
    sqlite3 $DATABASE "$STM"
}

#   no errors....
function insert_event2 {
    STM="INSERT INTO log ( reader, acc, runtime, ret_code, ressize ) VALUES ( '$1', '$2', '$3', '$4', '$5' )"
    sqlite3 $DATABASE "$STM"
}

function get_url() {
    ACC="$1"
    SDLURL="$SDL?acc=$ACC&location=sra-ncbi&location-type=forced"
    CURLRESPONSE=`curl -s $SDLURL`
    result=`echo $CURLRESPONSE | jq -r '.result[0].files[0].locations[0].link'`
    if [ "$result" == "null" ]; then
        echo "get_url( $ACC ) failed"
        insert_event2 'sdl' "$ACC" "-" "-" "1"
    fi
}

#   $1 ... reader
#   $2 ... acc
#   $3 ... runtime
#   $4 ... ret-code
function record_results() {
    RR_READER=$1
    RR_ACC=$2
    RR_RT=$3
    RR_RET=$4
    RR_SIZE=`ls -l $TEMPSTDOUT | awk '{ print $5 }'`
    if [ $RR_RET -ne 0 ]; then
        echo "$RR_READER $ACC failed"
        insert_event1 "$RR_READER" "$RR_ACC" "$RR_RT" "$RR_RET" "$RR_SIZE"
    else
        echo "$RR_READER $ACC succeeded"
        insert_event2 "$RR_READER" "$RR_ACC" "$RR_RT" "$RR_RET" "$RR_SIZE"
    fi
    rm -f $TEMPSTDOUT $TEMPSTDERR $TIMING
}

function fastq() {
    ACC="$1"
    get_url $ACC
    if [ ! "$result" == "null" ]; then
        URL="$result"
        echo "$ACC at $URL"
        /usr/bin/time -f %e -o $TIMING fastq-dump -Z $URL > $TEMPSTDOUT 2> $TEMPSTDERR
        RET="$?"
        RT=`cat $TIMING`
        record_results 'fastq-cloudian' "$ACC" "$RT" "$RET"
    fi
}

function fastq_special() {
    ACC="$1"
    URL="$SPECIAL_BASE$ACC.bits/$ACC"
    echo "$ACC at $URL"
    /usr/bin/time -f %e -o $TIMING fastq-dump -Z $URL > $TEMPSTDOUT 2> $TEMPSTDERR
    RET="$?"
    RT=`cat $TIMING`
    record_results 'fastq-spec-cloudian' "$ACC" "$RT" "$RET"
}

function fastq_vdb3() {
    ACC="$1"
    URL="$SPECIAL_BASE$ACC.bits/"
    echo "$ACC at $URL"
    /usr/bin/time -f %e -o $TIMING ./reader2.py -U $URL > $TEMPSTDOUT 2> $TEMPSTDERR
    RET="$?"
    RT=`cat $TIMING`
    record_results 'vdb3-cloudian' "$ACC" "$RT" "$RET"
}

function fastq_vdb3_aws() {
    ACC="$1"
    URL="$AWS_BASE$ACC.bits/"
    echo "$ACC at $URL"
    /usr/bin/time -f %e -o $TIMING ./reader2.py -U $URL > $TEMPSTDOUT 2> $TEMPSTDERR
    RET="$?"
    RT=`cat $TIMING`
    record_results 'vdb3-aws' "$ACC" "$RT" "$RET"
}

function fastq_vdb3_par() {
    ACC="$1"
    URL="$SPECIAL_BASE$ACC.bits2/"
    echo "$ACC at $URL"
    /usr/bin/time -f %e -o $TIMING ./reader2.py -U $URL -p > $TEMPSTDOUT 2> $TEMPSTDERR
    RET="$?"
    RT=`cat $TIMING`
    record_results 'vdb3-cloudina-par' "$ACC" "$RT" "$RET"
}

function fastq_sdl() {
    ACC="$1"
    URL=`srapath $ACC`
    echo "$ACC at $URL"
    /usr/bin/time -f %e -o $TIMING fastq-dump -Z $ACC > $TEMPSTDOUT 2> $TEMPSTDERR
    RET="$?"
    RT=`cat $TIMING`
    record_results 'fastq-sdl' "$ACC" "$RT" "$RET"
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
#export $RELIABLE

while true; do
    for acc in $ACCESSIONS; do
        exit_on_stopfile
        $SELECTION $acc
    done
    echo "now sleeping"
    sleep 20
done
