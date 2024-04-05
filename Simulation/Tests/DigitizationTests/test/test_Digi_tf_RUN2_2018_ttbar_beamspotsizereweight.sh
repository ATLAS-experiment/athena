#!/bin/bash
#
# art-description: Run digitization of an mc20e ttbar sample with 2018 geometry and conditions, 25ns pile-up
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-memory: 4096
# art-include: 24.0/Athena
# art-include: main/Athena
# art-output: mc16e_ttbar_beamspotsizereweight.*.RDO.pool.root

Events=25
DigiOutFileName="mc16e_ttbar_beamspotsizereweight.CG.RDO.pool.root"
InputHitsFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.simul.HITS.e4993_s3091/HITS.10504490._000425.pool.root.1"
HighPtMinbiasHitsFiles="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.361239.Pythia8EvtGen_A3NNPDF23LO_minbias_inelastic_high.merge.HITS.e4981_s3087_s3089/*"
LowPtMinbiasHitsFiles="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.361238.Pythia8EvtGen_A3NNPDF23LO_minbias_inelastic_low.merge.HITS.e4981_s3087_s3089/*"


Digi_tf.py \
    --CA \
    --conditionsTag default:OFLCOND-MC16-SDR-RUN2-09 \
    --digiSeedOffset1 170 --digiSeedOffset2 170 \
    --digiSteeringConf 'StandardSignalOnlyTruth' \
    --geometryVersion default:ATLAS-R2-2016-01-00-01 \
    --inputHITSFile ${InputHitsFile} \
    --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
    --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
    --jobNumber 568 \
    --maxEvents ${Events} \
    --outputRDOFile ${DigiOutFileName} \
    --postExec 'all:from IOVDbSvc.IOVDbSvcConfig import addOverride;cfg.merge(addOverride(flags, "/Indet/Beampos", "IndetBeampos-13TeV-MC16-002"))' \
    --postInclude 'PyJobTransforms.UseFrontier' 'HITtoRDO:DigitizationConfig.DigitizationSteering.DigitizationTestingPostInclude' \
    --preInclude 'HITtoRDO:Campaigns.MC20a' \
    --preExec 'HITtoRDO:flags.Digitization.InputBeamSigmaZ=42;' \
    --skipEvents 0

rc=$?
status=$rc
echo "art-result: $rc digiCA"

# get reference directory
source DigitizationCheckReferenceLocation.sh
echo "Reference set being used: ${DigitizationTestsVersion}"

rc4=-9999
if [[ $rc -eq 0 ]]
then
    # Do reference comparisons
    art.py compare ref --mode=semi-detailed --no-diff-meta "$DigiOutFileName" "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DigitizationTests/ReferenceFiles/$DigitizationTestsVersion/$CMTCONFIG/$DigiOutFileName"
    rc4=$?
    status=$rc4
fi
echo "art-result: $rc4 OLDvsFixedRef"

if [[ $rc -eq 0 ]]
then
    art.py compare grid --entries "$Events" "$1" "$2" --mode=semi-detailed --file=${DigiOutFileName}
    rc5=$?
    status=$rc5
fi
echo "art-result: $rc5 regression"

exit $status
