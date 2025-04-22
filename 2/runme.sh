#!/usr/bin/env bash

make
rm -f result.txt file.txt.lck 
for i in {1..10}; do
    ./file_locker -f file.txt &
done
sleep 300
killall -SIGINT file_locker

echo "All done. See result.txt"