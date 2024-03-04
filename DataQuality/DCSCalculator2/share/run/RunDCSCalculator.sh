#!/bin/bash

#  This script runs as a cron job in order to process new runs with the DCSCalculator2
#  It will normally process the runs listed in ./runList which is filled by ./GetNextRunList.py
#  If however, you wish to run the calculator manually on a set of runs (for fixes, etc.),
#  you may provide the the list of runs in a text file as argument to this script.
#  The text file must have one run number per line

prodPath=${DCSC_DATADIR:-/afs/cern.ch/user/a/atlasdqm/ws/DCSCalc/RunDCSCalc}
basePath=$(dirname "$0")
scriptLogPath=${DCSC_SCRIPTLOGDIR:-$prodPath/logfiles}
logPath=${DCSC_RUNLOGDIR:-$basePath/logfiles}

TMP=${TMPDIR:-/tmp}

if [[ $# -ne 0 ]]; then
    runList=$1
else
    runList=$prodPath/runList
fi

logPath=$prodPath/logfiles

if [ -e $runList ]
then

    echo "Using run list $runList"

    echo "Getting COOL authentications."
    export CORAL_AUTH_PATH={$CORAL_AUTH_PATH:-$HOME/private}
    
    # Get list of runs to loop over from runList (any chance of duplicates?)
    list=`cat $runList`
    
    for run in $list
    do
        echo "-----------------------------------------------------> NEW RUN!"
        echo "Run number = $run"
        echo "Running calculator..."

        # DCSCalculator2
        $basePath/ExecuteDCSC2.sh $run &> $logPath/dcsc2_$run
   
        # Append the recently processed run numbers to the end of a file which
        # keeps track of the last 200 runs which were processed.
        runsLog=$scriptLogPath/processedRuns_DCSOFL.log
        echo "$run" >> $runsLog
        sort -u $runsLog > $TMP/finishedRuns
        rm -f $runsLog
        lines=`wc -l $TMP/finishedRuns | awk '{print $1}'`
        if [ $lines -gt 200 ]
        then
            start=`expr $lines - 200`
            sed "1,$start d" $TMP/finishedRuns > $runsLog
        else
            cp $TMP/finishedRuns $runsLog
        fi
        rm -f $TMP/finishedRuns
        chmod g+w $runsLog
    
    done

else
    echo "runList file doesn't exist"

fi

echo "Finished"