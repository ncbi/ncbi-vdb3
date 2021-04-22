#$1 is the accession
#$2 is the base-name for temp. files

echo "creating the original fastq"
fastq-dump $1 --split-spot -Z > $2.orig.fastq

echo "creating a temp. dir with the blobs in it"
./writer2.py $1 -O $2.temp

echo "creating fastq from the temp-dir"
./reader2.py $2.temp -D > $2.copy.fastq

echo "diffing the results"
sort $2.orig.fastq > $2.orig.sorted.fastq
sort $2.copy.fastq > $2.copy.sorted.fastq
diff --brief -s $2.orig.sorted.fastq $2.copy.sorted.fastq
