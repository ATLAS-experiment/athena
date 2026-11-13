#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

NTHREADS=${1}
NEVENTS=${2}
DATAFILE="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/EVNT/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8514/EVNT.32288062._002040.pool.root.1"

# RUN2 setup
GEOTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN2)")
CONDTAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")
ignore_pattern=""


# Run the job
export TRF_ECHO=1;
export ATHENA_CORE_NUMBER=${NTHREADS};

Sim_tf.py \
      --maxEvents ${NEVENTS} \
      --perfmon 'fullmonmt' \
      --conditionsTag "default:${CONDTAG}" \
      --geometryVersion "default:${GEOTAG}" \
      --postInclude 'default:PyJobTransforms.UseFrontier' \
      --preInclude 'EVNTtoHITS:Campaigns.MC23eSimulationMultipleIoV' \
      --preExec "EVNTtoHITS:flags.Sim.FastCalo.ParamsInputFilename=\"FastCaloSim/MC23/TFCSparam_dev_Hybrid_D_v1_all_baryons_0_500.root\";flags.Detector.GeometryTRT=True;flags.LAr.doAlign=False;flags.GeoModel.Align.Dynamic=False;flags.Acts.TrackingGeometry.UseBlueprint=True;flags.Detector.GeometryCalo=True;flags.Acts.TrackingGeometry.MaterialCalibrationFolder='ACTS/MaterialMaps/ID';flags.Acts.TrackingGeometry.MaterialSource='material-maps-ATLAS-R2-2016-00-00-00.json'" \
      --postExec 'all:cfg.getService("AlgResourcePool").CountAlgorithmInstanceMisses = True' \
      --ignorePatterns "${ignore_pattern}" \
      --simulator 'ATLFAST3F_ACTSMT' \
      --inputEVNTFile ${DATAFILE} \
      --DataRunNumber 284500 \
      --imf False \
      --outputHITSFile 'myHITS.pool.root' > __log.txt 2>&1;

echo $? > __exitcode;


