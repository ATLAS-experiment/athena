#!/bin/bash

# Define the range
start=1
end=1280
#end=1000

# Loop through the range and check for missing files
for i in $(seq -f "%04g" $start $end); do
    filename="pwg-${i}-st2-stat.dat"
    if [[ ! -f "$filename" ]]; then
        echo "Missing file: $filename"
    fi
done
