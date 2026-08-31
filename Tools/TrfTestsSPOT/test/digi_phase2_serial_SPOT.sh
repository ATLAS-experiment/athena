#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

NEVENTS=${1}

CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")
GEOTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")

HSHitsFile="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-05-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8481_s4676/HITS.51318242._004216.pool.root.1"
HighPtMinbiasHitsFiles="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-05-00-00/mc21_14TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8481_s4676_s4677/*"
LowPtMinbiasHitsFiles="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN5-04-00-00/mc21_14TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8481_s4676_s4677/*"

conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

# Run the job
export TRF_ECHO=1;
Digi_tf.py \
   --perfmon 'fullmonmt' \
   --PileUpPresampling 'True' \
   --conditionsTag "${conditions}" \
   --digiSeedOffset1 '170' \
   --digiSeedOffset2 '170' \
   --digiSteeringConf 'StandardSignalOnlyTruth' \
   --inputHITSFile ${HSHitsFile} \
   --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
   --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles}  \
   --maxEvents ${NEVENTS} \
   --outputRDOFile 'myRDO.pool.root' \
   --postInclude 'PyJobTransforms.UseFrontier' \
   --preInclude 'HITtoRDO:Campaigns.MC23PhaseIIPileUp200' \
   --jobNumber '568' > __log.txt 2>&1;

echo $? > __exitcode;
