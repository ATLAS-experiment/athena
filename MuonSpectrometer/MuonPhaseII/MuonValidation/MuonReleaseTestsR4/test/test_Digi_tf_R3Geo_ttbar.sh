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
geo_db="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/ATLAS-R3-MUONTEST_v3.db"
geo_tag="ATLAS-R3S-2021-03-02-00"
validNTuple="MuonDigitNTuple.root"

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
        --postInclude 'all:PyJobTransforms.UseFrontier' \
        --preExec "default:flags.Scheduler.CheckDependencies = True;flags.Scheduler.ShowDataDeps = True;flags.Scheduler.ShowDataFlow = True;flags.Scheduler.ShowControlFlow = True;" \
        --postExec "default:flags.dump(evaluate=True);from MuonPRDTestR4.MuonHitTestConfig import MuonDigiTestCfg;cfg.merge(MuonDigiTestCfg(flags,dumpSimHits=True,dumpDigits=True, outFile=\"${validNTuple}\"));cfg.printConfig(withDetails=True, summariseProps=True);"

rc=$?
echo  "art-result: $rc digitization"
exit ${rc}
