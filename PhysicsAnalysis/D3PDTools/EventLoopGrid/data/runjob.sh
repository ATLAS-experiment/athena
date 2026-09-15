#!/bin/bash

if [ ! $# -eq 1 ] && [ ! $# -eq 3 ] ; then
    echo "$0 : Error: This script expects 1 or 3 arguments."
    exit 1
fi

command -v root > /dev/null 2>&1 || { 
    echo >&2 "$0 : Error: Root not set up." 
    exit 1 
}

unset ROOT_TTREECACHE_SIZE

SkipEvents=0
nEventsPerJob=-1

if [ $# -eq 3 ] ; then
    SkipEvents=$2
    nEventsPerJob=$3
fi

echo Executing eventloop_run_grid_job \"$1\" ${SkipEvents} ${nEventsPerJob}
date

eventloop_run_grid_job $1 ${SkipEvents} ${nEventsPerJob}
exitcode=$?

echo Finished executing root
date

exit $exitcode
