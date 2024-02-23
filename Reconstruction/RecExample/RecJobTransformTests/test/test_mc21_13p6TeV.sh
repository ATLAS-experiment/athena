#!/bin/sh
#
# art-description: Reco_tf runs on MC21a 13.6 TeV ttbar with HITS input (overlay+trigger+reco)
# art-output: log.*
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

export ATHENA_CORE_NUMBER=8
Reco_tf.py --CA --multithreaded --outputRDOFile=myRDO.pool.root --outputAODFile=myAOD.pool.root --outputESDFile=myESD.pool.root --inputHITSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/mc21_13p6TeV/HITSFiles/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8453_s3873/HITS.29625927._000632.pool.root.1 --inputRDO_BKGFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/OverlayTests/PresampledPileUp/22.0/Run3/v4/mc21a_presampling.VarBS.RDO.pool.root --maxEvents=300 --conditionsTag="OFLCOND-MC21-SDR-RUN3-07" --geometryVersion="ATLAS-R3S-2021-03-00-00" --preInclude 'Campaigns.MC21a'

RES=$?
echo "art-result: $RES Reco"
