#!/bin/bash

usage () {
    [ $# -gt 1 ] && echo $2
    echo "
    Command line script to run track reconstruction
    for the G-130 pipeline only as offline-like algorithms (Full-Scan)

    Usage:
    runReco_G130_FS.sh -i <your_input_RDO_file> -o <your_output_AOD_file_name>

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

run() {
  (
    set -x
    "$@"
  )
}

inputRDO=""
outputAOD=""
nEvents="-1"
skipCheck=0
storeTrackSeeds=True
numThreads=1

## parsing flags
while [ $# -ge 1 ];do
    case "$1" in
        --) shift ; break ;;
        -i  | --inputRDO )      if [ $# -lt 2 ] ; then usage ; fi ; inputRDO="$2"  ; shift ;;
        -o  | --outputAOD )     if [ $# -lt 2 ] ; then usage ; fi ; outputAOD="$2" ; shift ;;
        -n  | --nEvents )       if [ $# -lt 2 ] ; then usage ; fi ; nEvents="$2"   ; shift ;;
        -s  | --skipCheck )     if [ $# -lt 1 ] ; then usage ; fi ; skipCheck=1    ;;
        -t  | --noStoreSeeds )  if [ $# -lt 1 ] ; then usage ; fi ; storeTrackSeeds=False ;;
        -T  | --numThreads )    if [ $# -lt 2 ] ; then usage ; fi ; numThreads="$2" ; shift ;;
        -h  | --help )          usage 0 ;;
        *) shift ;;
    esac
    shift
done

## checking valid inputs
if [ -z $inputRDO ]; then usage ; fi
if [ -z $outputAOD ]; then usage ; fi


## running reconstruction
export PATHRESOLVER_DEVAREARESPONSE="WARNING"
conditionsTag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")
run Reco_tf.py \
    --conditionsTag "default:${conditionsTag}" \
    --maxEvents ${nEvents} \
    --preInclude 'InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude' \
    --postInclude 'ActsConfig.ActsPostIncludes.ACTSClusterPostInclude' \
    --preExec "flags.Detector.EnableHGTD=False; \
               flags.Acts.Device.doClusterization=True; \
               flags.Acts.Device.doSeeding=True; \
               flags.Tracking.doPixelDigitalClustering=True; \
               from ActsConfig.ActsConfigFlags import SeedingStrategy; \
               flags.Acts.Device.seedingStrategy=SeedingStrategy.Gbts; \
               flags.Acts.Gbts.connectionTable='binTables_ITK_RUN4_intraLayerLinks.txt'; \
               flags.Tracking.ITkActsPass.storeTrackSeeds=${storeTrackSeeds}; \
               flags.Concurrency.NumThreads=${numThreads}; \
               flags.Concurrency.NumConcurrentEvents=${numThreads};" \
    --inputRDOFile "${inputRDO}" \
    --outputAODFile "${outputAOD}" \
    --perfmon fullmonmt

rc=$?
echo "Reco_tf.py result: $rc"
# don't exit only for ERRORs detected in logfile (rc=68)
if [ $rc != 0 -a $rc != 68 ]; then exit $rc; fi

## check output
if [ "$skipCheck" == "0" ]; then
    checkxAOD.py ${outputAOD} > ${outputAOD}.checkxAOD.log
    checkFile.py ${outputAOD} > ${outputAOD}.checkFile.log
fi

exit $rc
