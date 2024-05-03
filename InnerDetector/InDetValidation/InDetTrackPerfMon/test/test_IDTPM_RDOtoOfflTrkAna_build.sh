#!/bin/bash
# art-description: Test to trasform RDO->AOD and then run IDTPM for an Offline TrackAnalysis
# art-input: mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700
# art-type: build
# art-include: main/Athena
# art-output: *.root

input_RDO=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1

nEvents=15

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
