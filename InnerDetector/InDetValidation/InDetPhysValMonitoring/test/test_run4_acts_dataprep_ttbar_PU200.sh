#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction, all-hadronic ttbar, full pileup, acts activated
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_shifter_last
# art-athena-mt: 8

lastref_dir=last_results
dcubeXml=dcube_IDPVMPlots_ACTS_R22.xml
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
    cmd=("${@:2}")
    ############
    echo "Running ${name}..."
    time "${cmd[@]}"
    rc=$?
    # Only report hard failures for comparison Acts-Trk since we know
    # they are different. We do not expect this test to succeed
    [ "${name}" = "dcube-trk" ] && [ $rc -ne 255 ] && rc=0
    echo "art-result: $rc ${name}"
    return $rc
}

# Run ACTS Reco
export ATHENA_CORE_NUMBER=8
ignore_pattern="Acts.+FindingAlg.+ERROR.+Propagation.+reached.+the.+step.+count.+limit,Acts.+FindingAlg.+ERROR.+Propagation.+failed:.+PropagatorError:..+Propagation.+reached.+the.+configured.+maximum.+number.+of.+steps.+with.+the.+initial.+parameters,Acts.+FindingAlg.Acts.+ERROR.+failed.+to.+extrapolate.+track"

run "Reconstruction-acts" \
    Reco_tf.py --CA \
    --inputRDOFile ${rdo_23p0} \
    --outputAODFile AOD.acts.root \
    --steering doRAWtoALL \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" \
    --postInclude "ActsConfig.ActsRegionsOfInterestConfig.ActsMainRegionsOfInterestCreatorAlgCfg,ActsConfig.ActsClusterizationConfig.ActsMainClusterizationCfg,ActsConfig.ActsSpacePointFormationConfig.ActsMainSpacePointFormationCfg,ActsConfig.ActsPostIncludes.PersistifyActsEDMCfg" \
    --preExec "flags.Reco.EnableHGTDExtension=False;flags.Acts.EDM.PersistifyClusters=True;flags.Acts.EDM.PersistifySpacePoints=True;" \
    --ignorePatterns "${ignore_pattern}" \
    --maxEvents ${nEvents} \
    --perfmon fullmonmt \
    --multithreaded

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

# Run Athena Reco
run "Reconstruction-athena" \
    Reco_tf.py --CA \
    --inputRDOFile ${rdo_23p0} \
    --outputAODFile AOD.athena.root \
    --steering doRAWtoALL \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" \
    --postInclude "InDetConfig.InDetPrepRawDataFormationConfig.ITkInDetToXAODClusterConversionCfg,InDetConfig.SiSpacePointFormationConfig.InDetToXAODSpacePointConversionCfg,ActsConfig.ActsPostIncludes.PersistifyActsEDMCfg" \
    --preExec "flags.Reco.EnableHGTDExtension=False;flags.Acts.EDM.PersistifyClusters=True;flags.Acts.EDM.PersistifySpacePoints=True;" \
    --maxEvents ${nEvents} \
    --perfmon fullmonmt \
    --multithreaded

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

run "IDPVM-acts" \
    runIDPVM.py \
    --filesInput AOD.acts.root \
    --outputFile idpvm.acts.root \
    --OnlyTrackingPreInclude \
    --doActs

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

run "IDPVM-athena" \
    runIDPVM.py \
    --filesInput AOD.athena.root \
    --outputFile idpvm.athena.root \
    --OnlyTrackingPreInclude \
    --doActs

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

echo "download latest result..."
art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
ls -la "$lastref_dir"

run "dcube-last" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_shifter_last \
    -c ${dcubeXmlAbsPath} \
    -r ${lastref_dir}/idpvm.acts.root \
    idpvm.acts.root

run "dcube-trk" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_trk \
    -c ${dcubeXmlAbsPath} \
    -r idpvm.athena.root \
    -M "acts" \
    -R "athena" \
    idpvm.acts.root
