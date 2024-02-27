#!/bin/sh
#
# art-description: Run MT and ST simulation outside ISF, reading ttbar events, writing HITS, using MC16 geometry and conditions
# art-include: 23.0/Athena
# art-include: main/Athena

# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-output: log.*
# art-output: test.MT.HITS.pool.root
# art-output: test.ST.HITS.pool.root

export ATHENA_CORE_NUMBER=8

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
    --outputHITSFile="test.MT.HITS.pool.root" \
    --physicsList="FTFP_BERT_ATL"               \
    --conditionsTag "default:OFLCOND-MC21-SDR-RUN3-07"          \
    --geometryVersion="default:ATLAS-R3S-2021-03-00-00"         \
    --imf False;
rc=$?
status=$rc
echo  "art-result: $rc MTsim"

unset ATHENA_CORE_NUMBER
AtlasG4_tf.py \
    --CA \
    --randomSeed '10' \
    --maxEvents '20' \
    --skipEvents '0' \
    --preInclude 'sim:Campaigns.MC21Simulation,SimuJobTransforms.FastCaloSim' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --postExec 'sim:cfg.addEventAlgo(CompFactory.JobOptsDumperAlg(FileName="CAConfig.txt"))' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
    --outputHITSFile="test.ST.HITS.pool.root" \
    --physicsList="FTFP_BERT_ATL"               \
    --conditionsTag "default:OFLCOND-MC21-SDR-RUN3-07"          \
    --geometryVersion="default:ATLAS-R3S-2021-03-00-00"         \
    --imf False;
rc2=$?
if [ $status -eq 0 ]
then
    status=$rc2
fi
echo  "art-result: $rc2 STsim"

rc3=-9999
if [ $status -eq 0 ]
then
    acmd.py diff-root test.MT.HITS.pool.root test.ST.HITS.pool.root --error-mode resilient --mode=semi-detailed --order-trees
    rc3=$?
    status=$rc3
fi
echo  "art-result: $rc3 comparision"

rc4=-9999
if [ $rc -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 20 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --order-trees --file=test.MT.HITS.pool.root
    rc4=$?
    if [ $status -eq 0 ]
    then
        status=$rc4
    fi
fi

echo  "art-result: $rc4 regression"

exit $status
