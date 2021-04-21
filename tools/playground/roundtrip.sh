#$1 is the accession
#$2 is the base-name for temp. files

ROWS=10000

echo "creating the original fastq"
fastq-dump $1 -X $ROWS -Z > $2.orig.fastq

echo "creating a temp. dir with the blobs in it"
./writer2.py $1 -N $ROWS -O $2.temp

echo "creating fastq from the temp-dir"
./reader2.py $2.temp > $2.copy.fastq

echo "diffing the results"
diff --brief -s $2.orig.fastq $2.copy.fastq
