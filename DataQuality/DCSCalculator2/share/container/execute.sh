#!/usr/bin/bash

INTERVAL=${INTERVAL:-30m}

while true
do
    ./GetNextRunList.py
    ./RunDCSCalculator.sh || exit 
    if [[ "$1" == "once" ]] ; then exit 0 ; fi
    sleep $INTERVAL
done