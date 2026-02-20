#!/usr/bin/bash
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

NEVENTS=${1}

CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")
GEOTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")

HSHitsFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.900149.PG_single_nu_Pt50.simul.HITS.e8481_s4149/HITS.33990267._000025.pool.root.1"
HighPtMinbiasHitsFiles="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8481_s4149_s4150/*"
LowPtMinbiasHitsFiles="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8481_s4149_s4150/*"

# Run the job
export TRF_ECHO=1;
Digi_tf.py \
   --perfmon 'fullmonmt' \
   --CA 'True'\
   --PileUpPresampling 'True' \
   --conditionsTag 'all:OFLCOND-MC21-SDR-RUN4-02' \
   --digiSeedOffset1 '170' \
   --digiSeedOffset2 '170' \
   --digiSteeringConf 'StandardSignalOnlyTruth' \
   --geometryVersion 'default:ATLAS-P2-RUN4-03-00-00' \
   --inputHITSFile ${HSHitsFile} \
   --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
   --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles}  \
   --maxEvents ${NEVENTS} \
   --outputRDOFile 'myRDO.pool.root' \
   --postInclude 'PyJobTransforms.UseFrontier' \
   --preInclude 'HITtoRDO:Campaigns.PhaseIIPileUp200' \
   --jobNumber '568' > __log.txt 2>&1;

echo $? > __exitcode;
