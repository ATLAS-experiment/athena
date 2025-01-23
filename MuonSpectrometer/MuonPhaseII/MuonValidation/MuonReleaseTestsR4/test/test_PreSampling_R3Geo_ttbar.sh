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
geo_db="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/ATLAS-R3-MUONTEST_v3.db"
geo_tag="ATLAS-R3S-2021-03-02-00"


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
	      --geometrySQLiteFullPath "${geo_db}" \
        --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-07'\
        --digiSeedOffset1 170 \
        --digiSeedOffset2 170 \
        --digiSteeringConf 'StandardSignalOnlyTruth' \
        --geometryVersion "default:${geo_tag}" \
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
