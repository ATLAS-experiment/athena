#!/usr/bin/bash

INTERVAL=${INTERVAL:-30m}

while true
do
    ./GetNextRunList.py
    ./RunDCSCalculator.sh
    sleep $INTERVAL
done