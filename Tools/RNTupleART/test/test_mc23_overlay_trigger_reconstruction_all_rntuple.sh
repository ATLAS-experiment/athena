#!/bin/bash
#
# art-description: Reco_tf.py MC23 Overlay+Trigger+Reconstruction in RNTuple Format
# art-type: grid
# art-include: main/Athena
# art-include: main--dev3LCG/Athena
# art-include: main--dev4LCG/Athena
# art-output: *.root
# art-output: log.*
# art-athena-mt: 8

HITS_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/HITS/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8514_s4162/100events.HITS.pool.root"
RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/RDO_BKG/mc23_13p6TeV.900149.PG_single_nu_Pt50.merge.RDO.e8514_e8528_s4153_d1907_d1908/100events.RDO.pool.root"
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")

# Convert the inputs first
HITSMerge_tf.py \
  --inputHITSFile="${HITS_File}" \
  --outputHITS_MRGFile="hits.rntuple.root" \
  --preExec="flags.PoolSvc.DefaultContainerType=\"ROOTRNTUPLE\";";

echo "art-result: $? hits-conversion";

RDOMerge_tf.py \
  --PileUpPresampling="True" \
  --inputRDOFile="${RDO_BKG_File}" \
  --outputRDO_MRGFile="rdo_bkg.rntuple.root" \
  --preExec="flags.PoolSvc.DefaultContainerType=\"ROOTRNTUPLE\";";

echo "art-result: $? rdobkg-conversion";

# Overlay+Trigger+Reconstruction
ATHENA_CORE_NUMBER=8 \
timeout 64800 \
Reco_tf.py \
  --CA="default:True" \
  --inputHITSFile="hits.rntuple.root" \
  --inputRDO_BKGFile="rdo_bkg.rntuple.root" \
  --outputAODFile="myAOD.pool.root" \
  --outputESDFile="myESD.pool.root" \
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
  --preExec="flags.PoolSvc.DefaultContainerType=\"ROOTRNTUPLE\";";
 
echo "art-result: $? full-chain";
