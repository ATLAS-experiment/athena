#!/bin/sh

# art-include: main--HepMC2/Athena
# art-include: main/Athena
# art-description: Pythia event generation -- min_bias
# art-type: grid

Gen_tf.py --ecmEnergy=13600. --maxEvents=100 --randomSeed=123456 --outputEVNTFile=EVNT.root --jobConfig=421113 --AMIConfig=e8462

echo "art-result: $? Gen_tf"

EVNTMerge_tf.py --inputEVNTFile=EVNT.root --maxEvent=100 --skipEvents=0 --outputEVNT_MRGFile=EVNT.MERGE_pool.root --AMIConfig=e8455

echo "art-result: $? ENVTMerge_tf"



Sim_tf.py --AMIConfig=s4369 --maxEvents=100 --inputEVNTFile=EVNT.MERGE_pool.root --outputHITSFile=HITS_421113.pool.root #--jobNumber=113

echo "art-result: $? Sim_tf"

HITSMerge_tf.py  --AMIConfig=s4370 --inputHITSFile=HITS_421113.pool.root --outputHITS_MRGFile=HITS_Mrg_421113_13p6.pool.root --maxEvents=100 --skipEvents=0

echo "art-results: $? HITSMerge_tf"



Reco_tf.py --AMIConfig=r16083 --inputHITSFile=HITS_Mrg_421113_13p6.pool.root --maxEvents=100 --jobNumber=113 --outputAODFile=AOD_421111_13p6.pool.root --inputRDO_BKGFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/RDO_BKG/mc23_13p6TeV.900149.PG_single_nu_Pt50.merge.RDO.e8514_e8528_s4332_s4324_d1994_d1943/100events.RDO.pool.root

echo "art-result: $? Reco_tf"

AODMerge_tf.py  --AMIConfig=r15970 --inputAODFile=AOD_421113.pool.root --outputAOD_MRGFile=AOD_merge_421113.pool.root --skipEvents=0

echo "art-result: $? AODMerge_tf"
~                                   
