
ROWS=10000
#PROTOBUF=-P
#PARALLEL=-p
#ACCESSION="SRR7392459"
ACCESSION="SRR8001010"

URL="-U https://sra-download.be-md.ncbi.nlm.nih.gov/sos3/vdb3testbucket/${ACCESSION}.bits2/"

# echo "creating the original fastq"
# fastq-dump $1 -Z > $2.orig.fastq

# echo "creating a temp. dir with the blobs in it"
# ./writer2.py $1 $PROTOBUF -O $2.temp

echo "creating fastq from the temp-dir"
echo "./reader2.py $URL $PROTOBUF $PARALLEL > ${ACCESSION}.copy.fastq"
./reader2.py $URL $PROTOBUF $PARALLEL > ${ACCESSION}.copy.fastq

#echo "diffing the results"
#diff --brief -s $2.orig.fastq $2.copy.fastq

