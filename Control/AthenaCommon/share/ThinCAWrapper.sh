#!/bin/sh
#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# Thin wrapper to execute configurations with the CA. 
# Might be called from athena.py
#

#Separate the top-level python script (ending with .py) from
#arguments passed to that script 
scriptargs=()
for a in "$@"
do
    case "$a" in
	--config-only*)  export PICKLECAFILE=${a#*=};;
	--tracelevel*) tracelevel=${a#*=};;
	--help) helpreq=1
		scriptargs+=("$a");;
	*.py) topscriptfile=$a;;
	*.pkl) picklefile=$a;;
	*) scriptargs+=("$a");;
    esac
done

#Check if we got a pickle-file or top-level script
if [ -z "${picklefile}" ] && [ -z "${topscriptfile}" ] ;then
        if [ -n "${helpreq}" ]; then
	    topscriptfile="AthenaConfiguration/AthNoop.py" #Dummy script to print help-text
	else   
	    echo "ERROR: No top-level python script or pickle file given"
	    exit 1
	fi
fi

#If script, try to find it locally or in PYTHONPATH
if [ -n "${topscriptfile}" ]; then
    if [ -f "${topscriptfile}" ]; then
        topscript="${topscriptfile}"
    else
        for pp in . ${PYTHONPATH//:/ }; do
	            if [ -f "${pp}/${topscriptfile}" ]; then
	                topscript="${pp}/${topscriptfile}"
	                break
	            fi
        done
    fi
    if [ -z "$topscript" ]; then
        echo "Could not find python script $topscriptfile"
        exit 1
    fi
fi


if [ -n "${tracelevel}" ]; then

    if [[ "$tracelevel" == "--tracelevel" ]]; then
       tracelevel="3" #Set to 3 (default) if no number given
    fi
    
    if [[ "$tracelevel" =~ ^[0-3]$ ]]; then
	echo "Trace level set to $tracelevel"
    else
	echo "ERROR, unexpected argument for --tracelevel, expect a number between 0 and 3, got ${tracelevel}"
	exit 1
    fi
	
    tracecmd=" -m trace --trace "
    if [ $tracelevel -gt 0 ]; then
	sysexclude=`python -m AthenaCommon.excludetracepath`
	tracecmd="${tracecmd} --ignore-dir ${sysexclude}"
    fi
    if  [ $tracelevel -gt 1 ]; then
	excludemods1="_db,__init__,_configurables,GaudiHandles,six"
	tracecmd="${tracecmd} --ignore-module ${excludemods1}"
    fi
    if [ $tracelevel -gt 2 ]; then
	excludemods2a=",Configurable,Configurables,PropertyProxy,DataHandle,ConfigurableDb,ConfigurableMeta,CfgMgr" #Legacy stuff we pull in b/c PythonAlgorithms and such still derive from old-style configurables
	excludemods2b=",ComponentAccumulator,Logging,AthConfigFlags,AllConfigFlags,Deduplication,semantics,AccumulatorCache,DebuggingContext,AtlasSemantics,ComponentFactory,LegacySupport,ItemListSemantics" #Internals of the ComponentAccumulator
	tracecmd="${tracecmd}${excludemods2a}${excludemods2b}"
    fi
    echo "Python tracing activated as ",$tracecmd
fi


#Finally: Execute it!
if [ -z "${picklefile}" ]; then
    python $tracecmd $topscript "${scriptargs[@]}"
else
    echo "Starting from pickle file $picklefile"
    CARunner.py $picklefile "${scriptargs[@]}"
fi
status=$?

#Handle exit code
if [ ! $status -eq 0 ]; then
    echo "leaving with code $status: \"failure\""
    exit $status
else
    echo "leaving with code 0: \"successful run\""
fi
