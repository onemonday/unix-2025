#!/bin/bash
make -s sparce
make -s create_A

./create_A
./sparce A B
./sparce A D -b 100

gzip -kf A
gzip -kf B

gzip -cd B.gz | ./sparce C

for file in A A.gz B B.gz C D; do
    if [ -e "$file" ]; then
        blocks=$(stat -c %b "$file")
        block_size=$(stat -c %B "$file")
        total=$((blocks * block_size))
        echo "file $file: $total bytes"
    else
        echo "file $file: not found" >&2
    fi
done > result.txt
