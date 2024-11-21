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
    cmd=("${@:2}")
    ############
    echo "Running ${name}..."
    time "${cmd[@]}"
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

run "IDPVM-athena" \
    runIDPVM.py \
    --filesInput AOD.athena.root \
    --outputFile idpvm.athena.root \
    --OnlyTrackingPreInclude \
    --doActs --doHGTD

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

run "Reconstruction-acts" \
    Reco_tf.py --CA \
    --inputRDOFile ${rdo_23p0} \
    --outputAODFile AOD.acts.root \
    --steering doRAWtoALL \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" \
    --postInclude "ActsConfig.ActsClusterizationConfig.ActsHgtdClusterizationAlgCfg,ActsConfig.ActsPostIncludes.PersistifyActsEDMCfg" \
    --preExec "flags.Reco.EnableHGTDExtension=True;flags.Acts.EDM.PersistifyClusters=True" \
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
    --doActs --doHGTD

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

run "Reconstruction-acts-timedclustering" \
    Reco_tf.py --CA \
	   --inputRDOFile ${rdo_23p0} \
	   --outputAODFile AOD.acts.timed.root \
	   --steering doRAWtoALL \
	   --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" \
	   --postInclude "ActsConfig.ActsClusterizationConfig.ActsHgtdClusterizationAlgCfg,ActsConfig.ActsPostIncludes.PersistifyActsEDMCfg" \
	   --preExec "flags.Reco.EnableHGTDExtension=True; \
    	       	      flags.Acts.EDM.PersistifyClusters=True; \
	       	      from HGTD_Config.HGTD_ConfigFlags import ClusteringStrategy; \
	       	      flags.HGTD.Acts.ClusteringStrategy=ClusteringStrategy.MultiPad; " \
	   --maxEvents ${nEvents} \
	   --perfmon fullmonmt \
	   --multithreaded

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

run "IDPVM-acts-timed" \
    runIDPVM.py \
    --filesInput AOD.acts.timed.root \
    --outputFile idpvm.acts.timed.root \
    --OnlyTrackingPreInclude \
    --doActs --doHGTD

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi


echo "download latest result..."
art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
ls -la "$lastref_dir"

run "dcube-last-acts" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_acts_shifter_last \
    -c ${dcubeXmlAbsPath} \
    -r ${lastref_dir}/idpvm.acts.root \
    idpvm.acts.root

run "dcube-last-acts-timed" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_acts_timed_shifter_last \
    -c ${dcubeXmlAbsPath} \
    -r ${lastref_dir}/idpvm.acts.timed.root \
    idpvm.acts.timed.root

run "dcube-last-athena" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_athena_shifter_last \
    -c ${dcubeXmlAbsPath} \
    -r ${lastref_dir}/idpvm.athena.root \
    idpvm.athena.root

run "dcube-athena-acts" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_athena_acts \
    -c ${dcubeXmlAbsPath} \
    -r idpvm.athena.root \
    -M "acts" \
    -R "athena" \
    idpvm.acts.root

run "dcube-acts-space-timed" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_acts_space_time \
    -c ${dcubeXmlAbsPath} \
    -r idpvm.acts.root \
    -R "Space_Matching" \
    -M "Space_And_Time_Matching" \
    idpvm.acts.timed.root
