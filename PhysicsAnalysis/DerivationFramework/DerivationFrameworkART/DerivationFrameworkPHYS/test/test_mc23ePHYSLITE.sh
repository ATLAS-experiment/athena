#!/bin/sh

# art-include: main/Athena
# art-description: DAOD building PHYSLITE mc23e
# art-type: grid
# art-memory: 4096
# art-input: user.martindl.mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_s4369_r16083
# art-input-nfiles: 1 
# art-output: *.pool.root
# art-output: checkFile*.txt
# art-output: checkxAOD*.txt
# art-output: checkIndexRefs*.txt

if [[ -z ${ArtInFile} ]]; then
    ArtInFile="root://eosatlas.cern.ch//eos/atlas/atlascerngroupdisk/data-art/large-input/CampaignInputs/mc23/AOD/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_s4369_r16083/AOD.41608496._001231.pool.root.1"
fi

condition=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")

set -e

Derivation_tf.py \
--inputAODFile ${ArtInFile} \
--outputDAODFile art.pool.root \
--conditionsTag ${condition} \
--formats PHYSLITE \
--maxEvents 1000 

echo "art-result: $? reco"

checkFile.py DAOD_PHYSLITE.art.pool.root > checkFile_PHYSLITE.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_PHYSLITE.art.pool.root > checkxAOD_PHYSLITE.txt

echo "art-result: $?  checkxAOD"

checkIndexRefs.py DAOD_PHYSLITE.art.pool.root > checkIndexRefs_PHYSLITE.txt 2>&1

echo "art-result: $?  checkIndexRefs"
