#!/bin/bash
# art-description: Test to trasform RDO->AOD and then run IDTPM for an Offline TrackAnalysis
# art-input: mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8514_s4345_r15583_tid39626672_00
# art-type: build
# art-include: main/Athena
# art-output: *.root

input_RDO=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-01/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8514_s4345_r15583_tid39626672_00/RDO.39626672._001121.pool.root.1

nEvents=10

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

run "Reconstruction" \
    Reco_tf.py --CA \
    --inputRDOFile ${input_RDO} \
    --outputAODFile AOD.root \
    --steering doRAWtoALL \
    --maxEvents ${nEvents} \
    --preInclude InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude

run "IDTPM" \
    runIDTPM.py \
    --inputFileNames AOD.root \
    --outputFilePrefix myIDTPM \
    --trkAnaCfgFile InDetTrackPerfMon/offlTrkAnaConfig.json \
    --writeAOD_IDTPM \
    --maxEvents ${nEvents} \
    --debug
