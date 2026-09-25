#!/bin/bash

NTHREADS=${1} 
NEVENTS=${2}

export GEOMODEL_DB_FILE=$(python -c "from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults; print(MuonPhaseIITestDefaults.GEODB_R4);")
export ATLAS_CONDDB_TAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")
export ATLAS_GEO_TAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN4)")
export HIT_FILES=$(python -c "from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults; print(\",\".join(MuonPhaseIITestDefaults.RDO_R4_MU200))")
export ATHENA_CORE_NUMBER=${NTHREADS}


echo ${HIT_FILES}

Reco_tf.py \
    --inputRDOFile="${HIT_FILES}" \
    --multithreaded True \
    --geometrySQLite True \
    --geometrySQLiteFullPath "${GEOMODEL_DB_FILE}" \
    --conditionsTag "default:${ATLAS_CONDDB_TAG}" \
    --preExec "all:flags.Common.MsgSuppression=True;flags.Scheduler.CheckDependencies=True;flags.Scheduler.ShowDataDeps=True;flags.Scheduler.ShowDataFlow=True;flags.Scheduler.ShowControlFlow = True;flags.Detector.EnablePLR=False;flags.Detector.EnableBCMPrime=False;flags.Acts.TrackingGeometry.UseBlueprint = True;flags.Acts.doLargeRadius=False;flags.Muon.scheduleActsReco = True;" \
    --postExec "default:flags.dump(evaluate=True);cfg.printConfig(withDetails=True, summariseProps=True);" \
    --postInclude 'all:PyJobTransforms.UseFrontier' \
    --outputESDFile myESD.pool.root \
    --outputAODFile myAOD.pool.root \
    --imf False \
    --maxEvents ${NEVENTS} \
    --perfmon "fullmonmt" \
    
 

