#!/bin/bash
# art-description: Test running pipeliene
# art-type: grid
# art-include: main/Athena
# art-memory: 8192
# art-output: *.txt
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_last


set -e
echo "$ArtInFile"
fileList="${ArtInFile// /,}"
echo $fileList
# PREFIX="F610"
lastref_dir=last_results

ATHENA_SOURCE="${ATLAS_RELEASE_BASE}/Athena/${Athena_VERSION}/InstallArea/${Athena_PLATFORM}/src/"
DCUBE_CONFIG="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/dcube_config_G4Validation.xml"



# Create HITS_SIM.pool.root file for latest release
setupATLAS
mkdir -p athena_latest
(
  cd athena_latest
  test_RUN3_FullG4MT_QS_ttbar_MT.sh
  SimValid_tf.py --inputHITSFile test.CA.HITS.pool.root --outputHIST_SIMFile test.CA.HITS_SIM.pool.root
)

# Create HITS_SIM.pool.root file for 25.0.47 release
setupATLAS
mkdir -p athena_25.0.47
(
  cd athena_25.0.47
  asetup Athena,25.0.47
  test_RUN3_FullG4MT_QS_ttbar_MT.sh
  SimValid_tf.py --inputHITSFile test.CA.HITS.pool.root --outputHIST_SIMFile test.CA.HITS_SIM.pool.root
)



X_FILE="athena_25.0.47/test.CA.HITS_SIM.pool.root"
R_FILE="athena_latest/test.CA.HITS_SIM.pool.root"


# Don't run if dcube config for nightly cmp is not found
if [ -z "$DCUBE_CONFIG" ]; then
    echo "art-result: 1 $DCUBE_CONFIG not found"
    exit 1
fi

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
        -p -x ${X_FILE} \
        # --plotopts=ratio \
        -c ${DCUBE_CONFIG} \
        # -M "${PREFIX}" \
        # -R "${PREFIX}-previous" \
        -r ${R_FILE} \
        # IDTPM.${PREFIX}.HIST.root
fi