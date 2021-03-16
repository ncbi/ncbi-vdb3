
ROWS=10000
PROTOBUF=-P

echo "creating the original fastq"
fastq-dump $1 -Z > $2.orig.fastq

echo "creating a temp. dir with the blobs in it"
./writer2.py $1 $PROTOBUF -O $2.temp

echo "creating fastq from the temp-dir"
./reader2.py $2.temp  $PROTOBUF > $2.copy.fastq

echo "diffing the results"
diff --brief -s $2.orig.fastq $2.copy.fastq
