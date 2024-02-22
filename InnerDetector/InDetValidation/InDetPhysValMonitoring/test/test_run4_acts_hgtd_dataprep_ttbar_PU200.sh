#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction, all-hadronic ttbar, full pileup, acts activated
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_last
# art-athena-mt: 8

lastref_dir=last_results
dcubeXml=dcube_IDPVMPlots_HGTD.xml
rdo_23p0=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1
nEvents=20

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
    echo "art-result: $rc ${name}"
    return $rc
}

# Run reconstruction with Athena HGTD clustering and converting HGTD clusters into xAOD format
export ATHENA_CORE_NUMBER=8
run "Reconstruction-athena" \
    Reco_tf.py --CA \
    --inputRDOFile ${rdo_23p0} \
    --outputAODFile AOD.athena.root \
    --steering doRAWtoALL \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" \
    --postInclude "InDetConfig.InDetPrepRawDataFormationConfig.HGTDInDetToXAODClusterConversionCfg,ActsConfig.ActsPostIncludes.PersistifyActsEDMCfg" \
    --preExec "flags.Reco.EnableHGTDExtension=True;flags.Acts.EDM.PersistifyClusters=True" \
    --maxEvents ${nEvents} \
    --perfmon fullmonmt \
    --multithreaded

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

run "IDPVM" \
    runIDPVM.py \
    --filesInput AOD.athena.root \
    --outputFile idpvm.athena.root \
    --doActs --doHGTD

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

echo "download latest result..."
art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
ls -la "$lastref_dir"

run "dcube-last-athena" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_last \
    -c ${dcubeXmlAbsPath} \
    -r ${lastref_dir}/idpvm.athena.root \
    idpvm.athena.root
