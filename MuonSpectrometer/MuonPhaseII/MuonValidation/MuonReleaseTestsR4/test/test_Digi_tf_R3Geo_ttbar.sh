#!/bin/bash
#
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 8
# art-description: Digitization R3 geometry test with ID + MS
# art-architecture:  '#x86_64-intel'
# art-output: log.*
# art-output: MuonDigitNTuple.root
# art-output: myRDO.pool.root



export ATHENA_PROC_NUMBER=8
export ATHENA_CORE_NUMBER=8

BASE_DIR="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/OverlayTests_R3/"
HITS_FILE="${BASE_DIR}/601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep/myHits.pool.root"


GEOMODEL_DB_FILE=$(python -c "from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults; print(MuonPhaseIITestDefaults.GEODB_R3)")
ATLAS_CONDDB_TAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")
ATLAS_GEO_TAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")


validNTuple="MuonDigitNTuple.root"

 Digi_tf.py \
        --CA \
        --inputHITSFile ${HITS_FILE} \
        --multithreaded True \
        --geometrySQLite True \
	--geometrySQLiteFullPath "${GEOMODEL_DB_FILE}" \
        --conditionsTag "default:${ATLAS_CONDDB_TAG} "\
        --geometryVersion "default:${ATLAS_GEO_TAG}" \
        --digiSeedOffset1 170 \
        --digiSeedOffset2 170 \
        --digiSteeringConf 'StandardSignalOnlyTruth' \
        --jobNumber 568 \
        --outputRDOFile myRDO.pool.root \
        --skipEvents 0 \
        --maxEvents 10  \
        --postInclude 'all:PyJobTransforms.UseFrontier' \
        --preExec "default:flags.Scheduler.CheckDependencies = True;flags.Scheduler.ShowDataDeps = True;flags.Scheduler.ShowDataFlow = True;flags.Scheduler.ShowControlFlow = True;" \
        --postExec "default:flags.dump(evaluate=True);from MuonPRDTestR4.MuonHitTestConfig import MuonDigiTestCfg;cfg.merge(MuonDigiTestCfg(flags,dumpSimHits=True,dumpDigits=True, outFile=\"${validNTuple}\"));cfg.printConfig(withDetails=True, summariseProps=True);"

rc=$?
echo  "art-result: $rc digitization"
exit ${rc}
