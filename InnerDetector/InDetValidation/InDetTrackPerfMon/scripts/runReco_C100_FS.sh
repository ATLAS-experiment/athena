#!/bin/bash

usage () {
    [ $# -gt 1 ] && echo $2
    echo "
    Command line script to run track reconstruction
    for the reference (C-000) pipeline only as offline-like algorithms (Full-Scan)

    Usage:
    runReco_C100_FS.sh -i <your_input_RDO_file> -o <your_output_AOD_file_name>

    Options:
    -i  |  --inputRDO       STRING      full path to input RDO file (mandatory)
    -o  |  --outputAOD      STRING      name of the output AOD file (mandatory)
    -n  |  --nEvents        INT         Number of events to run on (default = -1 aka All)
    -s  |  --skipCheck                  skip checks on output AOD file
    -h  |  --help                       this help
    "
    [ $# -gt 0 ] && exit $1
    exit 0
}

inputRDO=""
outputAOD=""
nEvents="-1"
skipCheck=0

## parsing flags
while [ $# -ge 1 ];do
    case "$1" in
        --) shift ; break ;;
        -i  | --inputRDO )      if [ $# -lt 2 ] ; then usage ; fi ; inputRDO="$2"  ; shift ;;
        -o  | --outputAOD )     if [ $# -lt 2 ] ; then usage ; fi ; outputAOD="$2" ; shift ;;
        -n  | --nEvents )       if [ $# -lt 2 ] ; then usage ; fi ; nEvents="$2"   ; shift ;;
        -s  | --skipCheck )     if [ $# -lt 1 ] ; then usage ; fi ; skipCheck=1    ; shift ;;
        -h  | --help )          usage 0 ;;
        *) shift ;;
    esac
    shift
done

## checking valid inputs
if [ -z $inputRDO ]; then usage ; fi
if [ -z $outputAOD ]; then usage ; fi

if [ ! -f $inputRDO ]; then
    echo "runReco_C100_FS.sh result: 1 ${inputRDO} not found"
    exit 1
fi

## running reconstruction
Reco_tf.py --CA \
    --maxEvents ${nEvents} \
    --preInclude 'InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsFastWorkflowFlags' \
    --preExec 'flags.Tracking.writeExtendedSi_PRDInfo=True' \
    --steering 'doRAWtoALL' \
    --inputRDOFile ${inputRDO} \
    --outputAODFile ${outputAOD}
#    --conditionsTag 'all:OFLCOND-MC21-SDR-RUN4-02' \
#    --geometryVersion 'all:ATLAS-P2-RUN4-03-00-00' \

rc=$?
echo "Reco_tf.py result: $rc"
if [ $rc != 0 ]; then exit $rc; fi

## check output
if [ "$skipCheck" == "0" ]; then
    checkxAOD.py ${outputAOD} > ${outputAOD}.checkxAOD.log
    checkFile.py ${outputAOD} > ${outputAOD}.checkFile.log
fi
