#!/bin/bash
#
# art-description: Reco_tf.py MC21 Overlay+Trigger+Reconstruction in RNTuple Format
# art-type: grid
# art-include: main--dev3LCG/Athena
# art-include: main--dev4LCG/Athena
# art-output: *.root
# art-output: log.*
# art-athena-mt: 8

HITS_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/HITS/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8453_s3873/50events.HITS.pool.root"
RDO_BKG_File="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/RDO_BKG/mc21_13p6TeV.900149.PG_single_nu_Pt50.digit.RDO.e8453_e8455_s3864_d1761/50events.RDO.pool.root"
NEVENTS="50"

# Overlay
ATHENA_CORE_NUMBER=8 \
timeout 64800 \
Reco_tf.py \
  --CA="True" \
  --inputHITSFile="${HITS_File}" \
  --inputRDO_BKGFile="${RDO_BKG_File}" \
  --outputRDOFile="myRDO.pool.root" \
  --maxEvents="${NEVENTS}" \
  --multithreaded="True" \
  --preInclude="all:Campaigns.MC21a" \
  --postInclude="default:PyJobTransforms.UseFrontier" \
  --skipEvents="0" \
  --autoConfiguration="everything" \
  --conditionsTag="default:OFLCOND-MC21-SDR-RUN3-07" \
  --geometryVersion="default:ATLAS-R3S-2021-03-00-00" \
  --runNumber="601229" \
  --digiSeedOffset1="511" \
  --digiSeedOffset2="727" \
  --steering="doOverlay" \
  --preExec="flags.Output.StorageTechnology.EventData=\"ROOTRNTUPLE\";";

echo "art-result: $? overlay";

# RDOtoRDOTrigger
ATHENA_CORE_NUMBER=8 \
timeout 64800 \
Reco_tf.py \
  --CA="True" \
  --inputRDOFile="myRDO.pool.root" \
  --outputRDO_TRIGFile="myRDO_TRIG.pool.root" \
  --multithreaded="True" \
  --preInclude="all:Campaigns.MC21a" \
  --postInclude="default:PyJobTransforms.UseFrontier" \
  --autoConfiguration="everything" \
  --conditionsTag="default:OFLCOND-MC21-SDR-RUN3-07" \
  --geometryVersion="default:ATLAS-R3S-2021-03-00-00" \
  --runNumber="601229" \
  --steering="doRDO_TRIG" \
  --preExec="flags.Output.StorageTechnology.EventData=\"ROOTRNTUPLE\";";

echo "art-result: $? trigger";

# Reconstruction
ATHENA_CORE_NUMBER=8 \
timeout 64800 \
Reco_tf.py \
  --CA="True" \
  --inputRDOFile="myRDO_TRIG.pool.root" \
  --outputAODFile="myAOD.pool.root" \
  --multithreaded="True" \
  --preInclude="all:Campaigns.MC23c" \
  --postInclude="default:PyJobTransforms.UseFrontier" \
  --autoConfiguration="everything" \
  --conditionsTag="default:OFLCOND-MC21-SDR-RUN3-07" \
  --geometryVersion="default:ATLAS-R3S-2021-03-00-00" \
  --runNumber="601229" \
  --preExec="flags.Output.StorageTechnology.EventData=\"ROOTRNTUPLE\";";

echo "art-result: $? reconstruction";
