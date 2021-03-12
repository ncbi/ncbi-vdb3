
ROWS=10000

#create the original fastq
fastq-dump $1 -X $ROWS -Z > $2.orig.fastq

#create a temp. dir with the blobs in it
./writer2.py $1 -N $ROWS -O $2.temp

#create fastq from the temp-dir
./reader2.py $2.temp > $2.copy.fastq

#diff the results
diff --brief -s $2.orig.fastq $2.copy.fastq
