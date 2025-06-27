#!/bin/bash

usage () {
    [ $# -gt 1 ] && echo $2
    echo "
    Command line script to run track reconstruction
    for the F-100 pipeline as offline-like algorithms (Full-Scan)

    Usage:
    F100.sh -i <your_input_RDO_file> -o <your_output_AOD_file_name>

    Options:
    -i  |  --inputRDO       STRING      full path to input RDO file (mandatory)
    -o  |  --outputAOD      STRING      name of the output AOD file (mandatory)
    -x  |  --xclbin         STRING      path to the xclbin that needs to be run
    -n  |  --nEvents        INT         Number of events to run on (default = -1 aka All)
    -s  |  --skipCheck                  skip checks on output AOD file
    -c  |  --doClusters                 persistify xAOD cluster and space point containers
    -h  |  --help                       this help
    "
    [ $# -gt 0 ] && exit $1
    exit 0
}

xclbinPath="/eos/project/a/atlas-eftracking/FPGA_compilation/FPGA_compilation_hw/F110/kernels.hw.xclbin"

# ttbar sample
inputRDO="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1"

#single muon sample
# inputRDO="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/ATLAS-P2-RUN4-03-00-00/RDO/reg0_singlemu.root"

outputAOD="AOD.root"

nEvents="100"
skipCheck=0
storeClusters=False
## parsing flags
while [ $# -ge 1 ];do
    case "$1" in
        --) shift ; break ;;
        -i  | --inputRDO )      if [ $# -lt 2 ] ; then usage ; fi ; inputRDO="$2"  ; shift ;;
        -o  | --outputAOD )     if [ $# -lt 2 ] ; then usage ; fi ; outputAOD="$2" ; shift ;;
        -x  | --xclbin )        if [ $# -lt 2 ] ; then usage ; fi ; xclbinPath="$2" ; shift ;;
        -n  | --nEvents )       if [ $# -lt 2 ] ; then usage ; fi ; nEvents="$2"   ; shift ;;
        -s  | --skipCheck )     if [ $# -lt 1 ] ; then usage ; fi ; skipCheck=1    ;;
        -c  | --doClusters )    storeClusters=True ;;
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
## running reconstruction

Reco_tf.py --CA \
    --maxEvents ${nEvents} \
    --preInclude 'InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateF100Flags,FPGATrackSimConfTools.FPGATrackSimDataPrepConfig.FPGATrackSimDataPrepFlagCfg,EFTrackingFPGAPipeline.F100IntegrationConfig.F100FlagsCfg' \
    --preExec "flags.Tracking.doTruth=True;flags.Tracking.ITkActsValidateF100Pass.doFPGATrackSim=False;\
                flags.Acts.EDM.PersistifyClusters=${storeClusters};flags.Acts.EDM.PersistifySpacePoints=${storeClusters};\
                flags.FPGADataPrep.xclbin=\"${xclbinPath}\"" \
    --postInclude "ActsConfig.ActsPostIncludes.ACTSClusterPostInclude" \
    --steering 'doRAWtoALL' \
    --inputRDOFile ${inputRDO} \
    --outputAODFile ${outputAOD}


rc=$?
echo "Reco_tf.py result: $rc"
if [ $rc != 0 ]; then exit $rc; fi

## check output
if [ "$skipCheck" == "0" ]; then
    checkxAOD.py ${outputAOD} > ${outputAOD}.checkxAOD.log
    checkFile.py ${outputAOD} > ${outputAOD}.checkFile.log
fi
