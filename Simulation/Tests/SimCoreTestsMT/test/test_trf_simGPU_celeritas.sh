#!/bin/bash
#
# art-description: Sim_tf.py Celeritas
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
OUTPUT="PhaseIISim.Celeritas"

export ATHENA_CORE_NUMBER=8

timeout 10800 AtlasG4_tf.py  \
  --maxEvents 100 \
  --multithreaded \
  --detectors 'Calo' \
  --preInclude 'AtlasG4Tf:Campaigns.PhaseIISimulation,SimulationConfig.disablePhotonRussianRoulette,SimulationConfig.disableNeutronRussianRoulette,SimulationConfig.disableFrozenShowersFCalOnly' \
  --conditionsTag "${conditionsTag}" \
  --geometryVersion 'default:ATLAS-P2-RUN4-04-00-00' \
  --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
  --physicsList 'FTFP_BERT_ATL_Celer' \
  --preExec 'flags.Sim.OptionalUserActionList+=[ "G4UserActions.G4UserActionsConfig.CelerOffloadToolCfg" ]; from SimulationConfig.SimEnums import CalibrationRun; flags.Sim.CalibrationRun=CalibrationRun.Off; flags.Sim.G4Commands+=[ "/celer/maxNumTracks 262144", "/celer/maxInitializers 524288", "/celer/secondaryStackFactor 2", "/celer/maxNumSteps 1000", "/celer/device/stackSize 32192", "/celer/device/heapSize 104857600" ]; flags.GeoModel.EMECStandard=True' \
  --postInclude 'PyJobTransforms.UseFrontier' \
  --imf False \
  --inputEVNTFile $INPUT \
  --randomSeed='2695' \
  --jobNumber 1 \
  --outputHITSFile $OUTPUT.HITS.pool.root
rc1=$?
echo "art-result: ${rc1} Sim_tf_celeritas"

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

# change back to AthSimulation,main--simGPU so that the dcube labels are correct
asetup AthSimulation,main--simGPU,latest

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

