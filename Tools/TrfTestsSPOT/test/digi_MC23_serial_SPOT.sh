#!/usr/bin/bash
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

NEVENTS=${1}

DATADIR="/eos/atlas/atlascerngroupdisk/proj-spot/spot-job-inputs/mc23_13p6TeV"
CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")
GEOTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")


# Run the job
export TRF_ECHO=1;
Digi_tf.py \
       --perfmon 'fullmonmt' \
       --CA 'True'\
       --PileUpPresampling 'True' \
       --conditionsTag ${CONDTAG} \
       --digiSeedOffset1 '232' \
       --digiSeedOffset2 '232' \
       --digiSteeringConf 'StandardSignalOnlyTruth' \
       --geometryVersion ${GEOTAG} \
       --inputHITSFile "${DATADIR}/HITS/mc23_13p6TeV.900149.PG_single_nu_Pt50.simul.HITS.e8514_e8528_s4153/HITS.33603023._002972.pool.root.1" \
       --inputHighPtMinbiasHitsFile "${DATADIR}/HITS/mc23_13p6TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8514_e8528_s4118_s4120/*" \
       --inputLowPtMinbiasHitsFile "${DATADIR}/HITS/mc23_13p6TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8514_e8528_s4117_s4120/*"  \
       --maxEvents ${NEVENTS} \
       --outputRDOFile 'myRDO.pool.root' \
       --postInclude 'PyJobTransforms.UseFrontier' \
       --preInclude 'HITtoRDO:Campaigns.MC23cSingleBeamspot' \
       --jobNumber '568' > __log.txt 2>&1;


echo $? > __exitcode;
