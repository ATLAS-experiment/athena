#!/bin/bash
#
# art-description: Digitization R3 geometry test with ID + MS
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-output: log.*
# art-output: MuonSimHitNtuple.root
# art-output: myRDO.pool.root



export ATHENA_PROC_NUMBER=8
export ATHENA_CORE_NUMBER=8

GEOMODEL_DB_FILE=$(python -c "from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults; print(MuonPhaseIITestDefaults.GEODB_R3)")
ATLAS_CONDDB_TAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")
ATLAS_GEO_TAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")



BASE_DIR="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/OverlayTests_R3/"
HITS_FILE="${BASE_DIR}/601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep/myHits.pool.root"



highPtMinBiasDir="${BASE_DIR}/800831.Py8EG_minbias_inelastic_highjetphotonlepton/"
lowPtMinBiasDir="${BASE_DIR}/900311.Epos_minbias_inelastic_lowjetphoton/"
neutrinoDir="${BASE_DIR}/900149.PG_single_nu_Pt50/"

highPtMinBiasFiles=""
for x in `ls ${highPtMinBiasDir}`;do
  ln -s ${highPtMinBiasDir}${x} ./
  if [ -n "${highPtMinBiasFiles}" ]; then
      highPtMinBiasFiles="${highPtMinBiasFiles},"
  fi
  highPtMinBiasFiles="${highPtMinBiasFiles}${x}"
done

lowPtMinBiasFiles=""
for x in `ls ${lowPtMinBiasDir}`;do
  ln -s ${lowPtMinBiasDir}${x} ./
  if [ -n "${lowPtMinBiasFiles}" ]; then
      lowPtMinBiasFiles="${lowPtMinBiasFiles},"
  fi
  lowPtMinBiasFiles="${lowPtMinBiasFiles}${x}"
done

neutrinoFiles=""

for x in `ls ${neutrinoDir}`;do
  ln -s ${neutrinoDir}${x} ./
  if [ -n "${neutrinoFiles}" ]; then
      neutrinoFiles="${neutrinoFiles},"
  fi
  neutrinoFiles="${neutrinoFiles}${x}"
done

echo ${highPtMinBiasFiles}
echo ${lowPtMinBiasFiles}
echo ${neutrinoFiles}

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
        --inputHighPtMinbiasHitsFile ${highPtMinBiasFiles} \
        --inputLowPtMinbiasHitsFile ${lowPtMinBiasFiles} \
        --postInclude 'all:PyJobTransforms.UseFrontier' \
        --preExec "default:flags.Scheduler.CheckDependencies = True;flags.Scheduler.ShowDataDeps = True;flags.Scheduler.ShowDataFlow = True;flags.Scheduler.ShowControlFlow = True;" \
        --postExec "default:flags.dump(evaluate=True);from MuonPRDTestR4.MuonHitTestConfig import MuonHitTesterCfg;cfg.merge(MuonHitTesterCfg(flags,dumpSimHits=True, outFile=\"${validNTuple}\"));cfg.printConfig(withDetails=True, summariseProps=True);"

rc=$?
echo  "art-result: $rc pile-up merging"
exit ${rc}
