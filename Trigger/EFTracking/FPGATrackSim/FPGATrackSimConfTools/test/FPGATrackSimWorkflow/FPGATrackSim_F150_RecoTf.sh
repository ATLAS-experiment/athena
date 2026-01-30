#!/bin/bash

usage () {
    [ $# -gt 1 ] && echo $2
    echo "
    Command line script to run track reconstruction
    for the F-150 pipeline as offline-like algorithms (Full-Scan)

    Usage:
    FPGATrackSim_F150_RecoTF.sh -i <your_input_RDO_file> -o <your_output_AOD_file_name> [options]

    Options:
    -i  |  --inputRDO           STRING      full path to input RDO file (mandatory)
    -o  |  --outputAOD          STRING      name of the output AOD file (mandatory)
    -n  |  --nEvents            INT         Number of events to run on (default = -1 aka All)
    -d  |  --skipEvents         INT         Number of events to skip at start (default = 0)
    -s  |  --skipCheck                      skip checks on output AOD file
    -k  |  --doSeeds                        persistify track seeds (default off)
    -w  |  --writeAdditionalOutputData      write extra FPGATrackSim outputs (default off)
    -r  |  --region             STRING      region list; e.g. \"[34,98,162]\" or \"10-60,!20\" or \"*\".
    -g  |  --keepHitsStrategy   INT         GenScan.keepHitsStrategy value (default = 2)
    -j  |  --doGNN                          toggle on the GNN pixel seeding configuration (default off)
    -h  |  --help                           this help

    Examples:
      FPGATrackSim_F150_RecoTF.sh -i /path/in.root -o AOD.root -r \"[34,98,162]\"
      FPGATrackSim_F150_RecoTF.sh -i /path/in.root -o AOD.root -r 34,98,162 -g 2
    "
    [ $# -gt 0 ] && exit $1
    exit 0
}

# Defaults
#inputRDO="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1"
inputRDO="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/ATLAS-P2-RUN4-03-00-00/RDO/rdo_singleMu_alleta.root"
outputAOD="AOD.root"
nEvents="-1"
skipCheck=0
doSeeds="0"
skipEvents=0
writeAdditionalOutputData="0"
regionList="[34, 98, 162, 226, 290, 354, 418, 482, 546, 610, 674, 738, 802, 866, 930, 994, 1058, 1122, 1186, 1250]"
keepHitsStrategy="2"   # NEW: user-settable via -g/--keepHitsStrategy
doGNN="0"

## parsing flags
while [ $# -ge 1 ]; do
    case "$1" in
        --) shift ; break ;;
        -i  | --inputRDO )      if [ $# -lt 2 ] ; then usage 1 "Missing value for --inputRDO"; fi ; inputRDO="$2"  ; shift ;;
        -o  | --outputAOD )     if [ $# -lt 2 ] ; then usage 1 "Missing value for --outputAOD"; fi ; outputAOD="$2" ; shift ;;
        -n  | --nEvents )       if [ $# -lt 2 ] ; then usage 1 "Missing value for --nEvents"; fi ; nEvents="$2"   ; shift ;;
        -d  | --skipEvents )    if [ $# -lt 2 ] ; then usage 1 "Missing value for --skipEvents"; fi ; skipEvents="$2" ; shift ;;
        -s  | --skipCheck )     skipCheck=1 ;;
        -k  | --doSeeds )       doSeeds="1" ;;
        -w  | --writeAdditionalOutputData ) writeAdditionalOutputData="1" ;;
        -r  | --region )        if [ $# -lt 2 ] ; then usage 1 "Missing value for --region"; fi ; regionList="$2" ; shift ;;
        -g  | --keepHitsStrategy ) if [ $# -lt 2 ] ; then usage 1 "Missing value for --keepHitsStrategy"; fi ; keepHitsStrategy="$2" ; shift ;;
        -j  | --doGNN )         doGNN="1" ;;
        -h  | --help )          usage 0 ;;
        *) shift ; continue ;;
    esac
    shift
done

## checking valid inputs
if [ -z "$inputRDO" ]; then usage 1 "Input RDO not provided"; fi
if [ -z "$outputAOD" ]; then usage 1 "Output AOD not provided"; fi

# Handle inputRDO patterns or check files
if [[ "$inputRDO" == *"*"* ]]; then
    inputRDO_arg="$inputRDO"
else
    IFS=',' read -ra FILES <<< "$inputRDO"
    for file in "${FILES[@]}"; do
        if [[ ! -f "$file" ]]; then
            echo "Error: File not found: $file"
            exit 1
        fi
    done
    inputRDO_arg="$inputRDO"
fi

export ATHENA_CORE_NUMBER=1
source FPGATrackSim_CommonEnv.sh
# Prepare preExec flags
preExecFlags="flags.Tracking.doPixelDigitalClustering=True;\
               flags.Trigger.FPGATrackSim.GenScan.keepHitsStrategy=${keepHitsStrategy};\
               flags.Tracking.ITkActsValidateF150Pass.storeTrackSeeds=${doSeeds};\
               flags.Trigger.FPGATrackSim.mapsDir=\"${MAPS_5L}\";\
               flags.Trigger.FPGATrackSim.regionList=${regionList};"

if [ "$writeAdditionalOutputData" == "0" ]; then
    preExecFlags="${preExecFlags}flags.Trigger.FPGATrackSim.writeAdditionalOutputData=False;"
fi

if [ "$doGNN" == "0" ]; then # Do GenScan Pixel Seeding
    preExecFlags="${preExecFlags}flags.Trigger.FPGATrackSim.Hough.genScan=True;flags.Trigger.FPGATrackSim.sampleType='skipTruth';"
    preExecFlags="${preExecFlags}flags.Trigger.FPGATrackSim.doOverlapRemoval=False;flags.Trigger.FPGATrackSim.doOverlapRemovalBetweenRegions=False;"
else # Do GNN Pixel Seeding
    preExecFlags="${preExecFlags}flags.Trigger.FPGATrackSim.Hough.genScan=False;\
                    flags.Trigger.FPGATrackSim.Hough.GNN=True;\
                    flags.Trigger.FPGATrackSim.GNN.moduleMapPath=\"${GNN_MODULE_MAP}\";\
                    flags.Trigger.FPGATrackSim.GNN.MLModelPath=\"${GNN_METRIC_LEARNING}\";\
                    flags.Trigger.FPGATrackSim.GNN.GNNModelPath=\"${GNN_ONNX_MODEL}\";\
                    flags.Trigger.FPGATrackSim.GNN.moduleMapTol=0.5;\
                    flags.Trigger.FPGATrackSim.GNN.edgeScoreCut=0.5;\
                    flags.Trigger.FPGATrackSim.GNN.doGNNPixelSeeding=True;\
                    flags.Trigger.FPGATrackSim.doOverlapRemoval=False;\
                    flags.Trigger.FPGATrackSim.doOverlapRemovalBetweenRegions=False;\
                    flags.Trigger.FPGATrackSim.sampleType='singleMuons';"
fi

Reco_tf.py --CA \
    --maxEvents ${nEvents} \
    --skipEvents ${skipEvents} \
    --preInclude 'InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateF150Flags,FPGATrackSimConfTools.FPGATrackSimAnalysisConfig.FPGATrackSimF150FlagCfg' \
    --preExec "${preExecFlags}" \
    --postInclude "ActsConfig.ActsPostIncludes.ACTSClusterPostInclude" \
    --steering 'doRAWtoALL' \
    --inputRDOFile "${inputRDO_arg}" \
    --outputAODFile ${outputAOD}

rc=$?
echo "Reco_tf.py result: $rc"
if [ $rc != 0 ]; then exit $rc; fi

if [ "$skipCheck" == "0" ]; then
    checkxAOD.py ${outputAOD} > ${outputAOD}.checkxAOD.log
    checkFile.py ${outputAOD} > ${outputAOD}.checkFile.log
fi
