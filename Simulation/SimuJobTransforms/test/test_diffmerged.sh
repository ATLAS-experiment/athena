#!/bin/sh
#
# art-description: MC23-style simulation, then merge two copies of the HITS file
# art-type: build
# art-include: 23.0/Athena
# art-include: main/Athena

# MC23 setup
# ATLAS-R3S-2021-03-02-00 and OFLCOND-MC23-SDR-RUN3-08
export TRF_ECHO=1
export GEOMETRY=ATLAS-R3S-2021-03-02-00
export CONDITIONS=OFLCOND-MC23-SDR-RUN3-08

Sim_tf.py \
--inputEVNTFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/mu_E200_eta0-25.evgen.pool.root \
--outputHITSFile mu_E200_eta0-25_${GEOMETRY}.HITS.pool.root \
--maxEvents 5 \
--skipEvents 0 \
--geometryVersion ${GEOMETRY} \
--physicsList 'FTFP_BERT_ATL' \
--simulator 'FullG4MT_QS' \
--postInclude 'default:PyJobTransforms/UseFrontier.py' \
--preInclude 'EVNTtoHITS:Campaigns/MC23SimulationSingleIoV.py' \
--conditionsTag ${CONDITIONS}

echo  "art-result: $? simulation"
cp log.EVNTtoHITS log.EVNTtoHITS1

Sim_tf.py \
--inputEVNTFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/mu_E200_eta0-25.evgen.pool.root \
--outputHITSFile mu_E200_eta0-25_${GEOMETRY}.2.HITS.pool.root \
--maxEvents 5 \
--skipEvents 5 \
--geometryVersion ${GEOMETRY} \
--physicsList 'FTFP_BERT_ATL' \
--simulator 'FullG4MT_QS' \
--postInclude 'default:PyJobTransforms/UseFrontier.py' \
--preInclude 'EVNTtoHITS:Campaigns/MC23SimulationSingleIoV.py' \
--conditionsTag ${CONDITIONS}

echo  "art-result: $? simulation2"
cp log.EVNTtoHITS log.EVNTtoHITS2

INPUTFILE=mu_E200_eta0-25_${GEOMETRY}.HITS.pool.root
FILENAME=`basename ${INPUTFILE}`
FILEBASE=${FILENAME%.HITS.pool.root}
INPUTFILE2=${FILEBASE}.2.HITS.pool.root
RDOFILE1=${FILEBASE}.unmerged.RDO.pool.root
RDOFILE2=${FILEBASE}.merged.RDO.pool.root


INPUTLIST=$INPUTFILE,$INPUTFILE2
MERGEHITSFILE=${FILEBASE}.merged.HITS.pool.root
echo $INPUTLIST

INPUTLOGLIST=log.EVNTtoHITS1,log.EVNTtoHITS2

HITSMerge_tf.py \
--inputHITSFile "$INPUTLIST" \
--inputLogsFile "$INPUTLOGLIST" \
--outputHITS_MRGFile $MERGEHITSFILE \
--maxEvents 10 \
--skipEvents 0 \
--geometryVersion ${GEOMETRY}

echo  "art-result: $? mergeHITS"

Digi_tf.py \
--inputHITSFile $MERGEHITSFILE \
--outputRDOFile $RDOFILE2 \
--maxEvents 2 \
--skipEvents 6 \
--geometryVersion ${GEOMETRY} \
--conditionsTag ${CONDITIONS} \
--postExec 'HITtoRDO:condSeq.TileSamplingFractionCondAlg.G4Version = -1;'

echo  "art-result: $? mergeDigi"

Digi_tf.py \
--inputHITSFile "$INPUTLIST" \
--outputRDOFile $RDOFILE1 \
--maxEvents 2 \
--skipEvents 6 \
--geometryVersion ${GEOMETRY} \
--conditionsTag ${CONDITIONS} \
--postExec 'HITtoRDO:condSeq.TileSamplingFractionCondAlg.G4Version = -1;'

echo  "art-result: $? unmergeDigi"

diffPoolFiles.py $RDOFILE2 $RDOFILE1 | \
    sed 's/\[ERR\]\(.*\)POOLContainer_DataHeaderForm$/\[WARN\]\1POOLContainer_DataHeaderForm/g' | \
    sed 's/## Comparison : \[ERR\]/## Comparison : \[WARN\]/g'

echo  "art-result: $? diffRDO"
