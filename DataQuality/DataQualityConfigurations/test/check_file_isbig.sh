#!/bin/bash
# Check if the merged hanconfig file has a minimal size.

[[ $(wc -l $1 | awk '{ print $1 }') -gt $2 ]] && exit 0
echo "$1 has less than $2 lines. This might point to a problem in the hanconfig merging."
exit 1
