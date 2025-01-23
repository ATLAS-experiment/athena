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
geo_db="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/ATLAS-R3-MUONTEST_v3.db"
geo_tag="ATLAS-R3S-2021-03-02-00"
validNTuple="MuonDigitNTuple.root"

Overlay_tf.py \
    --CA \
    --multithreaded True \
    --geometrySQLite True \
    --geometrySQLiteFullPath "${geo_db}" \
    --runNumber 601229 \
    --inputHITSFile ${HITS_FILE} \
    --inputRDO_BKGFile ${RDO_BKG_File} \
    --outputRDOFile myRDO.pool.root \
    --maxEvents ${events} \
    --digiSeedOffset1 511 \
    --digiSeedOffset2 727 \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-07'\
    --geometryVersion "default:${geo_tag}" \
    --preInclude 'all:Campaigns.MC23a' \
    --preExec "default:flags.Scheduler.CheckDependencies=True;flags.Scheduler.ShowDataDeps=True;flags.Scheduler.ShowDataFlow=True;flags.Scheduler.ShowControlFlow=True" \
    --postExec "default:flags.dump(evaluate = True);from MuonPRDTestR4.MuonHitTestConfig import MuonDigiTestCfg;cfg.merge(MuonDigiTestCfg(flags,dumpSimHits=True,dumpDigits=True, outFile=\"${validNTuple}\"));cfg.getEventAlgo('EventInfoOverlay').ValidateBeamSpot = False;cfg.printConfig(withDetails=True, summariseProps=True);" \
    --imf False

