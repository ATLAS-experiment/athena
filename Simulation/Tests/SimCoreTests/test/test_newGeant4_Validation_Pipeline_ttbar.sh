#!/bin/bash
# art-description: Test running pipeline
# art-type: grid
# art-include: main/Athena
# art-memory: 8192
# art-cores: 8
# art-runtime: 86400
# art-output: *.txt
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_last


set -e
echo "$ArtInFile"
fileList="${ArtInFile// /,}"
echo $fileList

DCUBE_CONFIG="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/dcube_config_G4Validation.xml"
INPUT_EVNT_FILE="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1"


# Create HITS_SIM.pool.root file for latest release
mkdir -p athena_latest
(
    cd athena_latest

    export ATHENA_CORE_NUMBER=8
    geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")
    conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")


    # ttbar test
    Sim_tf.py \
        --CA \
        --multithreaded \
        --conditionsTag "default:${conditions}" \
        --geometryVersion "default:${geometry}" \
        --simulator 'FullG4MT_QS' \
        --postInclude 'PyJobTransforms.UseFrontier' \
        --preInclude 'EVNTtoHITS:Campaigns.MC23eSimulationMultipleIoV' \
        --inputEVNTFile ${INPUT_EVNT_FILE} \
        --outputHITSFile "test.CA.HITS.pool.root" \
        --maxEvents 1000 \
        --jobNumber 1 \
        --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
        --imf False

    SimValid_tf.py --inputHITSFile test.CA.HITS.pool.root --outputHIST_SIMFile test.CA.HITS_SIM.pool.root
)



R_FILE="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/athena_25.0.47_newGeant4_Validation_ttbar.root"
X_FILE="athena_latest/test.CA.HITS_SIM.pool.root"


# Don't run if dcube config for nightly cmp is not found
if [ -z "$DCUBE_CONFIG" ]; then
    echo "art-result: 1 $DCUBE_CONFIG not found"
    exit 1
fi

# Define a function to run a command and check its return code
run () {
    name="${1}"
    cmd="${@:2}"
    echo "Running ${name}..."
    time ${cmd}
    rc=$?
    echo "art-result: $rc ${name}"
    if [ $rc != 0 ]; then
        exit $rc
    fi
    return $rc
}

art.py download --user=artprod --dst=last_results "$ArtPackage" "$ArtJobName"
run "dcube-latest" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
        -p -x dcube_last \
        -c ${DCUBE_CONFIG} \
        -r ${R_FILE} \
        ${X_FILE}
status=$rc
echo "art-result: $? plots"
exit $status