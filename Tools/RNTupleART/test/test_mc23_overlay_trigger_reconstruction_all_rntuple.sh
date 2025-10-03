#!/bin/bash
#
# art-description: Reco_tf.py MC23 Overlay+Trigger+Reconstruction in RNTuple Format
# art-type: grid
# art-include: main--dev3LCG/Athena
# art-output: *.root
# art-output: log.*
# art-athena-mt: 8

HITS_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/HITS/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8514_s4162/100events.HITS.pool.root"
RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/RDO_BKG/mc23_13p6TeV.900149.PG_single_nu_Pt50.merge.RDO.e8514_e8528_s4153_d1907_d1908/100events.RDO.pool.root"
NEVENTS="-1"
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")

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
  --conditionsTag="default:${conditions}" \
  --geometryVersion="default:ATLAS-R3S-2021-03-02-00" \
  --runNumber="601237" \
  --digiSeedOffset1="232" \
  --digiSeedOffset2="232" \
  --AMITag="r14799" \
  --steering "doOverlay" "doRDO_TRIG" "doTRIGtoALL" \
  --preExec="flags.Output.StorageTechnology.EventData=\"ROOTRNTUPLE\";flags.Output.StorageTechnology.MetaData=\"ROOTRNTUPLE\";flags.PoolSvc.DefaultContainerType=\"ROOTRNTUPLE\";";
 
echo "art-result: $? full-chain";
