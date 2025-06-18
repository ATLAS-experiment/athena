#!/bin/bash

usage () {
    [ $# -gt 1 ] && echo $2
    echo "
    Command line script to run track reconstruction
    for the F-100 pipeline as offline-like algorithms (Full-Scan)

    Usage:
    FPGATrackSim_F100_RecoTF.sh -i <your_input_RDO_file> -o <your_output_AOD_file_name>

    Options:
    -i  |  --inputRDO       STRING      full path to input RDO file (mandatory)
    -o  |  --outputAOD      STRING      name of the output AOD file (mandatory)
    -n  |  --nEvents        INT         Number of events to run on (default = -1 aka All)
    -s  |  --skipCheck                  skip checks on output AOD file
    -c  |  --doClusters                 persistify xAOD cluster and space point containers
    -h  |  --help                       this help
    "
    [ $# -gt 0 ] && exit $1
    exit 0
}

inputRDO="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1"
outputAOD="AOD.root"
nEvents="1"
skipCheck=0
doClusters="0"

## parsing flags
while [ $# -ge 1 ];do
    case "$1" in
        --) shift ; break ;;
        -i  | --inputRDO )      if [ $# -lt 2 ] ; then usage ; fi ; inputRDO="$2"  ; shift ;;
        -o  | --outputAOD )     if [ $# -lt 2 ] ; then usage ; fi ; outputAOD="$2" ; shift ;;
        -n  | --nEvents )       if [ $# -lt 2 ] ; then usage ; fi ; nEvents="$2"   ; shift ;;
        -s  | --skipCheck )     if [ $# -lt 1 ] ; then usage ; fi ; skipCheck=1    ;;
        -c  | --doClusters )    if [ $# -lt 1 ] ; then usage ; fi ; doClusters="1" ;;
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
export ATHENA_CORE_NUMBER=1
source FPGATrackSim_CommonEnv.sh
## running reconstruction
if [ "$doClusters" == "1" ]; then
  Reco_tf.py --CA \
    --maxEvents ${nEvents} \
    --preInclude 'InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateF100Flags,FPGATrackSimConfTools.FPGATrackSimDataPrepConfig.FPGATrackSimDataPrepFlagCfg' \
    --preExec "flags.Trigger.FPGATrackSim.mapsDir=\"${MAPS_5L}\";flags.Tracking.doTruth=True;\
              flags.Acts.EDM.PersistifyClusters=True;flags.Acts.EDM.PersistifySpacePoints=True;" \
    --steering 'doRAWtoALL' \
    --inputRDOFile ${inputRDO} \
    --outputAODFile ${outputAOD}
else
  Reco_tf.py --CA \
    --maxEvents ${nEvents} \
    --preInclude 'InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateF100Flags,\
                  FPGATrackSimConfTools.FPGATrackSimDataPrepConfig.FPGATrackSimDataPrepFlagCfg' \
    --preExec "flags.Trigger.FPGATrackSim.mapsDir=\"${MAPS_5L}\";flags.Tracking.doTruth=True;"\
    --steering 'doRAWtoALL' \
    --inputRDOFile ${inputRDO} \
    --outputAODFile ${outputAOD}
fi

rc=$?
echo "Reco_tf.py result: $rc"
if [ $rc != 0 ]; then exit $rc; fi

## check output
if [ "$skipCheck" == "0" ]; then
    checkxAOD.py ${outputAOD} > ${outputAOD}.checkxAOD.log
    checkFile.py ${outputAOD} > ${outputAOD}.checkFile.log
fi
