#!/bin/bash
#
# art-description: Sim_tf.py Adept
# art-type: grid
# art-include: main--simGPU/AthSimulation
# art-athena-mt: 8
# art-architecture: '#&nvidia'
# art-output: dcube*
# art-html: dcube_simGPU

conditionsTag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

echo "--- modinfo ---"
modinfo nvidia | grep -i version
echo "--- nvidia-smi ---"
which nvidia-smi
echo "--- nvidia-smi ---"
nvidia-smi
echo "--- lscpu ---"
lscpu
echo "--- which nvcc ---"
which nvcc
echo "-----"
# Choose GPU with lowest utilization
export CUDA_VISIBLE_DEVICES=$(nvidia-smi --query-gpu=memory.free,index --format=csv,nounits,noheader | sort -nr | head -1 | awk '{ print $NF }')
echo "GPU with lowest utilization: $CUDA_VISIBLE_DEVICES"

echo "--- athena ---"
INPUT="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1"
OUTPUT="PhaseIISim.AdePT"

export ATHENA_CORE_NUMBER=8

timeout 64800 AtlasG4_tf.py  \
  --maxEvents 100 \
  --multithreaded \
  --simulator 'AtlasG4_QS' \
  --conditionsTag "${conditionsTag}" \
  --geometryVersion 'default:ATLAS-P2-RUN4-04-00-00' \
  --preInclude 'AtlasG4Tf:Campaigns.PhaseIISimulation' \
  --preExec 'flags.Sim.G4Commands+=["/adept/CallUserTrackingAction true", "/adept/CallUserSteppingAction false","/adept/setCovfieBfieldFile /cvmfs/atlas.cern.ch/repo/sw/database/GroupData/MagneticFieldMaps/bmagatlas_09_fullAsym20400_forGPU_v1.cvf", "/adept/setVerbosity 0", "/adept/addGPURegion EMB", "/adept/addGPURegion EMEC", "/adept/addGPURegion HEC", "/adept/addGPURegion PreSampLAr", "/adept/setTrackInAllRegions false", "/adept/setMillionsOfTrackSlots 4", "/adept/setMillionsOfHitSlots 7", "/adept/setCUDAStackLimit 32192", "/adept/setCUDAHeapLimit 84857600"];flags.GeoModel.EMECStandard=True' \
  --physicsList 'FTFP_BERT_ATL_AdePT' \
  --postInclude 'PyJobTransforms.UseFrontier' \
  --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
  --imf False \
  --inputEVNTFile $INPUT \
  --randomSeed='2695' \
  --jobNumber 1 \
  --outputHITSFile $OUTPUT.HITS.pool.root
rc1=$?
echo "art-result: ${rc1} Sim_tf_adept"

files=( $OUTPUT.HITS.pool.root )
for i in "${files[@]}"
do
    if [ -f "$i" ]; then
        rcFile=0
    else 
        rcFile=1
    fi
    echo "art-result: ${rcFile} $i exists"
done

# change to Athena,main since SimValid_tf does not work correctly in AthSimulation
export ATLAS_LOCAL_ROOT_BASE="/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase"
source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
asetup Athena,main,latest

echo "============ SimValid_tf.py"
SimValid_tf.py --inputHITSFile $OUTPUT.HITS.pool.root --outputHIST_SIMFile $OUTPUT.HIST.root --detectors LAr
rc4=$?
echo "art-result: ${rc4} SimValid_tf" 

echo "============ dcube reference file creation == local version == will be changed in next iteration"
$ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py -vv -g -r $OUTPUT.HIST.root -c dcube_config_hist_${OUTPUT}_25050.xml
rcref=$?
echo "art-result: ${rcref} create dcube reference " 

echo "============ dcube references == copy to CVMFS later == will be changed in next iteration"
dcubeRef="${OUTPUT}.HIST.root"
dcubeXML="dcube_config_hist_${OUTPUT}_25050.xml"
echo ${dcubeRef}
echo ${dcubeXML}

# Run dcube comparison
echo "============ dcube"
$ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py -p --jobId simGPUTest -c ${dcubeXML} -r ${dcubeRef} -x dcube_simGPU $OUTPUT.HIST.root
rc5=$?
echo "art-result: ${rc5} dcube_simGPU" 

