#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction, Single muon 100GeV, acts activated
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_ambi_last

lastref_dir=last_results
dcubeXml=dcube_IDPVMPlots_ACTS_CKF_ITk.xml
rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.900498.PG_single_muonpm_Pt100_etaFlatnp0_43.recon.RDO.e8481_s4149_r14697/RDO.33675668._000016.pool.root.1
nEvents=1000

# search in $DATAPATH for matching file
dcubeXmlAbsPath=$(find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 1 -name $dcubeXml -print -quit 2>/dev/null)
# Don't run if dcube config not found
if [ -z "$dcubeXmlAbsPath" ]; then
    echo "art-result: 1 dcube-xml-config"
    exit 1
fi

run () {
    name="${1}"
    cmd="${@:2}"
    ############
    echo "Running ${name}..."
    time ${cmd}
    rc=$?
    # Only report hard failures for comparison Acts-Trk since we know
    # they are different. We do not expect this test to succeed
    [ "${name}" = "dcube-ckf-ambi" ] && [ $rc -ne 255 ] && rc=0
    echo "art-result: $rc ${name}"
    return $rc
}

# Run w/o ambi. resolution
run "Reconstruction-ckf" \
    Reco_tf.py --CA \
    --steering doRAWtoALL \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateTracksFlags" \
    --inputRDOFile ${rdo} \
    --outputAODFile AOD.ckf.root \
    --maxEvents ${nEvents}

reco_rc=$?

# Rename log
mv log.RAWtoALL log.RAWtoALL.CKF

if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi


run "IDPVM" \
    runIDPVM.py \
    --filesInput AOD.ckf.root \
    --outputFile idpvm.ckf.root

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

# Run w/ ambi. resolution
run "Reconstruction-ambi" \
    Reco_tf.py --CA \
    --steering doRAWtoALL \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateResolvedTracksFlags" \
    --inputRDOFile ${rdo} \
    --outputAODFile AOD.ambi.root \
    --perfmon fullmonmt \
    --maxEvents ${nEvents}

reco_rc=$?

# Rename log
mv log.RAWtoALL log.RAWtoALL.AMBI

if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

run "IDPVM" \
    runIDPVM.py \
    --filesInput AOD.ambi.root \
    --outputFile idpvm.ambi.root

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

echo "download latest result..."
art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
ls -la "$lastref_dir"

run "dcube-ckf-last" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_ckf_last \
    -c ${dcubeXmlAbsPath} \
    -r ${lastref_dir}/idpvm.ckf.root \
    idpvm.ckf.root

run "dcube-ambi-last" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_ambi_last \
    -c ${dcubeXmlAbsPath} \
    -r ${lastref_dir}/idpvm.ambi.root \
    idpvm.ambi.root

# Compare performance w/ and w/o ambi. resolution
run "dcube-ckf-ambi" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_ckf_ambi \
    -c ${dcubeXmlAbsPath} \
    -r idpvm.ckf.root \
    idpvm.ambi.root
