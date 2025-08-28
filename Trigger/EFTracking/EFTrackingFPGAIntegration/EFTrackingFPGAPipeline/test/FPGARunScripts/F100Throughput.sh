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
    -p  |  --nPixelCU       INT         nPixelCU
    -s  |  --nStripCU       INT         nStripCU
    -b  |  --bdfid          STRING      bdfid of the FPGA to run on
    -f  |  --runF110                    run F110 Integration algo
    -h  |  --help                       this help
    "
    [ $# -gt 0 ] && exit $1
    exit 0
}

xclbinPath="/eos/project/a/atlas-eftracking/FPGA_compilation/FPGA_compilation_hw/F110/kernels.hw.xclbin"
bdfid="0000:c3:00.1"

# ttbar sample
inputRDO="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1"

#single muon sample
# inputRDO="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/ATLAS-P2-RUN4-03-00-00/RDO/reg0_singlemu.root"

outputAOD="AOD.root"

nEvents="100"
storeClusters=False
runF110=False
threads=1
nproc=0
doCodeType="F100"

## parsing flags
while [ $# -ge 1 ];do
    case "$1" in
        --) shift ; break ;;
        -i  | --inputRDO )      if [ $# -lt 2 ] ; then usage ; fi ; inputRDO="$2"  ; shift ;;
        -o  | --outputAOD )     if [ $# -lt 2 ] ; then usage ; fi ; outputAOD="$2" ; shift ;;
        -x  | --xclbin )        if [ $# -lt 2 ] ; then usage ; fi ; xclbinPath="$2" ; shift ;;
        -n  | --nEvents )       if [ $# -lt 2 ] ; then usage ; fi ; nEvents="$2"   ; shift ;;
        -b  | --bdfid )         if [ $# -lt 2 ] ; then usage ; fi ; bdfid="$2" ; shift ;;
        -t  | --threads )       if [ $# -lt 2 ] ; then usage ; fi ; threads="$2" ; shift ;;
        -r  | --procs )         if [ $# -lt 2 ] ; then usage ; fi ; nproc="$2" ; shift ;;
        -q  | --doCodeType )    if [ $# -lt 2 ] ; then usage ; fi ; doCodeType="$2" ; shift ;;
        -f  | --runF110 )       runF110=True ;;
        -h  | --help )          usage 0 ;;
        *) shift ;;
        esac
        shift
    done

## checking valid inputs
if [ -z "$inputRDO" ]; then usage ; fi
if [ -z "$outputAOD" ]; then usage ; fi

if [[ "$inputRDO" == *"*"* ]]; then
    # Just pass the pattern as is to Reco_tf.py in case of regex-like input
    inputRDO_arg="$inputRDO"
else
    # Check existence for comma-separated files
    IFS=',' read -ra FILES <<< "$inputRDO"
    for file in "${FILES[@]}"; do
        if [[ ! -f "$file" ]]; then
            echo "Error: File not found: $file"
            exit 1
        fi
    done
    inputRDO_arg="$inputRDO"
fi

## running reconstruction
echo "running local"

ATHENA_CORE_NUMBER=${threads} Reco_tf.py --CA \
    --maxEvents ${nEvents} \
    --preInclude 'InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateF100Flags,FPGATrackSimConfTools.FPGATrackSimDataPrepConfig.FPGATrackSimDataPrepFlagCfg,EFTrackingFPGAPipeline.F100IntegrationConfig.F100FlagsCfg' \
    --preExec "flags.Tracking.doTruth=True;flags.Tracking.ITkActsValidateF100Pass.doFPGATrackSim=False;\
                flags.Tracking.doTruth=False; flags.Output.doGEN_AOD2xAOD=False; flags.Reco.PostProcessing.GeantTruthThinning=False; \
                flags.Acts.EDM.PersistifyClusters=${storeClusters};flags.Acts.EDM.PersistifySpacePoints=${storeClusters};flags.Tracking.doVertexFinding=False;\
                flags.Concurrency.NumProcs=${nproc}; flags.Concurrency.NumConcurrentEvents=${threads}; flags.Concurrency.NumThreads=${threads}; flags.Output.AODFileName=\"\"; flags.Output.doWriteAOD=False;\
                flags.FPGADataPrep.doCodeType=\"${doCodeType}\";flags.FPGADataPrep.doF110=${runF110};flags.FPGADataPrep.bdfID=\"${bdfid}\";flags.FPGADataPrep.xclbin=\"${xclbinPath}\"" \
    --perfmon 'fullmonmt' \
    --autoConfiguration 'everything' \
    --multithreaded 'True' \
    --steering 'doRAWtoALL' \
    --inputRDOFile ${inputRDO_arg} \
    --outputAODFile ${outputAOD}

