#!/bin/bash
#
# art-description: Simulation test with R4 MS geometry + ITK
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 8
# art-architecture:  '#x86_64-intel'
# art-output: log.*
# art-output: MuonSimHitNtuple.root
# art-output: SimHitsR4.pool.root


GEOMODEL_DB_FILE=$(python -c "from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults; print(MuonPhaseIITestDefaults.GEODB_R4)")
ATLAS_CONDDB_TAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")
ATLAS_GEO_TAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN4)")

validNTuple="MuonSimHitNtuple.root"

export ATHENA_PROC_NUMBER=8
export ATHENA_CORE_NUMBER=8
Sim_tf.py \
      --CA True \
      --multithreaded True \
      --geometrySQLite True \
      --geometrySQLiteFullPath "${GEOMODEL_DB_FILE}" \
      --conditionsTag "default:${ATLAS_CONDDB_TAG} "\
      --geometryVersion "default:${ATLAS_GEO_TAG}" \
      --simulator 'FullG4MT_QS' \
      --postInclude 'PyJobTransforms.TransformUtils.UseFrontier' \
      --preExec "all:flags.Scheduler.CheckDependencies = True;flags.Scheduler.ShowDataDeps = True;flags.Scheduler.ShowDataFlow = True;flags.Scheduler.ShowControlFlow = True;" \
      --postExec "all:flags.dump(evaluate=True);from MuonPRDTestR4.MuonHitTestConfig import MuonHitTesterCfg;cfg.merge(MuonHitTesterCfg(flags,dumpSimHits=True, outFile=\"${validNTuple}\"));cfg.printConfig(withDetails=True, summariseProps=True);" \
      --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1' \
      --outputHITSFile 'SimHitsR4.pool.root' \
      --maxEvents 100 \
      --skipEvents 0 \
      --randomSeed 10 \
      --imf False

rc=$?
echo  "art-result: $rc simulation"

exit ${rc}
