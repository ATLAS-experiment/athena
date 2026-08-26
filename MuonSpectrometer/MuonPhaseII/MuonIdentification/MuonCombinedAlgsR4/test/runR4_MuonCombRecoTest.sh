#!/bin/bash



export GEOMODEL_DB_FILE=$(python -c "from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults; print(MuonPhaseIITestDefaults.GEODB_R4);")
export ATLAS_CONDDB_TAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")
export ATLAS_GEO_TAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN4)")
export RDO_FILES=$(python -c "from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults; print(\",\".join(MuonPhaseIITestDefaults.RDO_R4))")
export ATHENA_CORE_NUMBER=16

echo ${RDO_FILES}

Reco_tf.py \
    --CA  \
    --maxEvents -1 \
    --inputRDOFile="${RDO_FILES}" \
    --multithreaded True \
    --geometrySQLite True \
    --geometrySQLiteFullPath "${GEOMODEL_DB_FILE}" \
    --conditionsTag "default:${ATLAS_CONDDB_TAG}" \
    --preInclude "all:ActsConfig.ActsCIFlags.actsProductionFlags" \
    --preExec "all:flags.Common.MsgSuppression=False;flags.Scheduler.CheckDependencies=True;flags.Scheduler.ShowDataDeps=True;flags.Scheduler.ShowDataFlow=True;flags.Scheduler.ShowControlFlow = True;flags.Detector.EnablePLR=False;flags.Detector.EnableBCMPrime=False;flags.Acts.TrackingGeometry.UseBlueprint = True;" \
    --postExec "default:flags.dump(evaluate=True);from MuonTrackFindingTest.MsTrackFindingTester import MsTrackTesterCfg;cfg.merge(MsTrackTesterCfg(flags));cfg.printConfig(withDetails=True, summariseProps=True);cfg.getService('MessageSvc').setFatal=['copyAuxStoreThinned']" \
    --postInclude 'all:PyJobTransforms.UseFrontier' \
    --outputESDFile myESD.pool.root \
    --outputAODFile myAOD.pool.root \
    --imf False
    
 

