#!/bin/sh
#
# art-description: Tests implementation of FastCaloSimV2 as a Geant4 fast simulation engine with Run 3 geometry and conditions
# art-include: 23.0/Athena
# art-include: main/Athena
# art-athena-mt: 8
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: test.HITS.pool.root

export ATHENA_CORE_NUMBER=8

AtlasG4_tf.py \
    --multithreaded \
    --randomSeed '10' \
    --maxEvents '20' \
    --skipEvents '0' \
    --preInclude 'SimulationJobOptions/preInclude.FastCaloSim.py,Campaigns/MC21Simulation.py' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --postExec 'sim:topSeq.BeamEffectsAlg.ISFRun=True;topSeq+=CfgMgr.JobOptsDumperAlg(FileName="LegacyConfig.txt")' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
    --outputHITSFile="test.HITS.pool.root" \
    --physicsList="FTFP_BERT_ATL"               \
    --conditionsTag "default:OFLCOND-MC21-SDR-RUN3-07"          \
    --geometryVersion="default:ATLAS-R3S-2021-03-00-00"         \
    --imf False;

rc=$?
status=$rc
echo  "art-result: $rc simOLD"
cp log.AtlasG4Tf log.AtlasG4Tf_OLD

AtlasG4_tf.py \
    --CA \
    --multithreaded \
    --randomSeed '10' \
    --maxEvents '20' \
    --skipEvents '0' \
    --preInclude 'sim:Campaigns.MC21Simulation,SimuJobTransforms.FastCaloSim' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --postExec 'sim:cfg.addEventAlgo(CompFactory.JobOptsDumperAlg(FileName="CAConfig.txt"))' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
    --outputHITSFile="test.HITS.CA.pool.root" \
    --physicsList="FTFP_BERT_ATL"               \
    --conditionsTag "default:OFLCOND-MC21-SDR-RUN3-07"          \
    --geometryVersion="default:ATLAS-R3S-2021-03-00-00"         \
    --imf False;

rc1=$?
if [ $status -eq 0 ]
then
    status=$rc1
fi
echo  "art-result: $rc1 simCA"
cp log.AtlasG4Tf log.AtlasG4Tf_CA

rc4=-9999
if [ $rc -eq 0 ] && [ $rc1 -eq 0 ]
then
    acmd.py diff-root test.HITS.pool.root test.HITS.CA.pool.root --error-mode resilient --mode=semi-detailed --order-trees
    rc4=$?
    status=$rc4
fi
echo  "art-result: $rc4 HITS_OLDvsCA"



rc2=-9999
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 20 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --order-trees
    rc2=$?
    if [ $status -eq 0 ]
    then
        status=$rc2
    fi
fi

echo  "art-result: $rc2 regression"

exit $status
