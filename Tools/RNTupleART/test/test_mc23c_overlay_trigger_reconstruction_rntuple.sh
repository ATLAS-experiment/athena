#!/bin/bash
#
# art-description: Reco_tf.py MC23c Overlay+Trigger+Reconstruction in RNTuple Format
# art-type: grid
# art-include: main/Athena
# art-include: main--dev3LCG/Athena
# art-include: main--dev4LCG/Athena
# art-output: *.root
# art-output: log.*
# art-athena-mt: 8

HITS_File="/eos/atlas/atlascerngroupdisk/proj-spot/spot-job-inputs/mc23_13p6TeV/HITS/mc23_13p6TeV.601237.PhPy8EG_A14_ttbar_hdamp258p75_allhad.merge.HITS.e8514_e8528_s4159_s4114/HITS.34124871._003416.pool.root.1"
RDO_BKG_File="/eos/atlas/atlascerngroupdisk/proj-spot/spot-job-inputs/mc23_13p6TeV/RDO/mc23_13p6TeV.900149.PG_single_nu_Pt50.merge.RDO.e8514_e8528_s4153_d1879_d1880/RDO.33837536._002942.pool.root.1"
NEVENTS="1000"

# Overlay+Trigger+Reconstruction
ATHENA_CORE_NUMBER=8 \
timeout 64800 \
Reco_tf.py \
  --CA="default:True" \
  --inputHITSFile="${HITS_File}" \
  --inputRDO_BKGFile="${RDO_BKG_File}" \
  --outputAODFile="myAOD.pool.root" \
  --outputESDFile="myESD.pool.root" \
  --maxEvents="${NEVENTS}" \
  --multithreaded="True" \
  --preInclude="all:Campaigns.MC23c" \
  --postInclude="default:PyJobTransforms.UseFrontier" \
  --skipEvents="0" \
  --autoConfiguration="everything" \
  --conditionsTag="default:OFLCOND-MC23-SDR-RUN3-05" \
  --geometryVersion="default:ATLAS-R3S-2021-03-02-00" \
  --runNumbe="601237" \
  --digiSeedOffset1="232" \
  --digiSeedOffset2="232" \
  --AMITag="r14799" \
  --steering "doOverlay" "doRDO_TRIG" "doTRIGtoALL" \
  --preExec="flags.Output.StorageTechnology.EventData=\"ROOTRNTUPLE\";";
 
echo "art-result: $? full-chain";
