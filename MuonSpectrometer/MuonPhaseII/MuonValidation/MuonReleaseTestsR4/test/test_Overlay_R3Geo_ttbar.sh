#!/bin/bash
#
# art-description: Digitization R3 geometry test with ID + MS
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 8
# art-architecture:  '#x86_64-intel'
# art-output: log.*
# art-output: MuonDigitNTuple.root
# art-output: myRDO.pool.root

events=20
BASE_DIR="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/OverlayTests_R3/"
HITS_FILE="${BASE_DIR}/601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep/myHits.pool.root"
RDO_BKG_File="${BASE_DIR}/MinBias.RDO.pool.root"
validNTuple="MuonDigitNTuple.root"

GEOMODEL_DB_FILE=$(python -c "from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults; print(MuonPhaseIITestDefaults.GEODB_R3)")
ATLAS_CONDDB_TAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")
ATLAS_GEO_TAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")


Overlay_tf.py \
    --CA \
    --multithreaded True \
    --geometrySQLite True \
    --geometrySQLiteFullPath "${GEOMODEL_DB_FILE}" \
    --conditionsTag "default:${ATLAS_CONDDB_TAG} "\
    --geometryVersion "default:${ATLAS_GEO_TAG}" \
    --runNumber 601229 \
    --inputHITSFile ${HITS_FILE} \
    --inputRDO_BKGFile ${RDO_BKG_File} \
    --outputRDOFile myRDO.pool.root \
    --maxEvents ${events} \
    --digiSeedOffset1 511 \
    --digiSeedOffset2 727 \
    --preInclude 'all:Campaigns.MC23a' \
    --preExec "default:flags.Scheduler.CheckDependencies=True;flags.Scheduler.ShowDataDeps=True;flags.Scheduler.ShowDataFlow=True;flags.Scheduler.ShowControlFlow=True" \
    --postExec "default:flags.dump(evaluate = True);from MuonPRDTestR4.MuonHitTestConfig import MuonDigiTestCfg;cfg.merge(MuonDigiTestCfg(flags,dumpSimHits=True,dumpDigits=True, outFile=\"${validNTuple}\"));cfg.getEventAlgo('EventInfoOverlay').ValidateBeamSpot = False;cfg.printConfig(withDetails=True, summariseProps=True);" \
    --imf False

