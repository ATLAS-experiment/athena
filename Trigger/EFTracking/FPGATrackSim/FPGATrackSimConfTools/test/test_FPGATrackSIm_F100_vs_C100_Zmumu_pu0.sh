#!/bin/bash
# art-description: Compare F100 to C100 on Zmumu pu0 events (full detector)
# art-type: grid
# art-include: main/Athena
# art-memory: 16384
# art-input: mc21_14TeV:mc21_14TeV.601190.PhPy8EG_AZNLO_Zmumu.recon.RDO.e8481_s4203_r14697
# art-input-nfiles: 11
# art-output: *.txt
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_compare


set -e
echo "$ArtInFile"
fileList="${ArtInFile// /,}"
echo $fileList
PREFIX_F100="F100"
INPUT_AOD_FILE_F100="xAOD_${PREFIX_F100}.root"

PREFIX_C100="C100"

ATHENA_SOURCE="${ATLAS_RELEASE_BASE}/Athena/${Athena_VERSION}/InstallArea/${Athena_PLATFORM}/src/"
IDTPM_CONFIG="${ATHENA_SOURCE}/Trigger/EFTracking/FPGATrackSim/FPGATrackSimConfTools/test/IDTPM_configs/IDTPM_ttbar_allRegions.json"
DCUBE_CONFIG="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/ATLAS-P2-RUN4-03-00-00/dcube/config/FPGATrackSimIDTPMconfig.xml"

# Don't run if dcube config for nightly cmp is not found
if [ -z "$DCUBE_CONFIG" ]; then
    echo "art-result: 1 $DCUBE_CONFIG not found"
    exit 1
fi

# Don't run if IDTPM config for nightly is not found
if [ -z "$IDTPM_CONFIG" ]; then
    echo "IDTPM config $IDTPM_CONFIG not found"
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

# Run F100 and produce IDTPM output
run "${PREFIX_F100} pipeline" \
    FPGATrackSim_F100.sh -o $INPUT_AOD_FILE_F100 -t -n 10000 -q -i $fileList > F100.txt 2>&1 &



# Run C100 and produce IDTPM output
run "${PREFIX_C100} pipeline" \
    source ${ATHENA_SOURCE}/Trigger/EFTracking/FPGATrackSim/FPGATrackSimConfTools/test/FPGATrackSimWorkflow/FPGATrackSim_CommonEnv.sh -t -n 10000 -q -i $fileList
    python -m FPGATrackSimConfTools.C100Config \
        --evtMax="${RDO_EVT_ANALYSIS}" \
        --filesInput="${RDO_ANALYSIS}" \
        PhysVal.IDTPM.trkAnaCfgFile="${IDTPM_CONFIG}" \
        PhysVal.IDTPM.outputFilePrefix="IDTPM.${PREFIX_C100}" > C100.txt 2>&1 &

wait 

run "IDTPM" \
    runIDTPM.py --inputFileNames=$INPUT_AOD_FILE_F100 \
                --outputFilePrefix="IDTPM.${PREFIX_F100}" \
                --trkAnaCfgFile="${IDTPM_CONFIG}"

# Run dcube
run "dcube-F100_vs_C100" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
        -p -x dcube_compare \
        --plotopts=ratio \
        -c ${DCUBE_CONFIG} \
        -M "${PREFIX_F100}" \
        -R "${PREFIX_C100}" \
        -r IDTPM.${PREFIX_C100}.HIST.root \
        IDTPM.${PREFIX_F100}.HIST.root