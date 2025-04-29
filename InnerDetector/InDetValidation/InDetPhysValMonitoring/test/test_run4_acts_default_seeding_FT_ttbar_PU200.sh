#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction, all-hadronic ttbar, full pileup, acts activated
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_shifter_last

lastref_dir=last_results
dcubeXml=dcube_ART_ACTS_SEEDS_FT_R22.xml
input_rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1

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

# Only schedule what we need for this specific test
# We want to run Athena
# We want to schedule Trk->xAOD SP convertion
# We want to run the Acts Seeding Algorithm
# We want to activate the analysis of seed and estimated track parameters (flag)
run "Reconstruction" \
    Reco_tf.py \
    --inputRDOFile ${input_rdo} \
    --outputAODFile AOD.pool.root \
    --steering doRAWtoALL \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" \
    --postInclude "InDetConfig.SiSpacePointFormationConfig.InDetToXAODSpacePointConversionCfg,ActsConfig.ActsSeedingConfig.ActsPixelSeedingAlgCfg,ActsConfig.ActsAnalysisConfig.ActsPixelSeedsToTrackParamsAlgCfg,ActsConfig.ActsAnalysisConfig.ActsPixelSeedAnalysisAlgCfg,ActsConfig.ActsAnalysisConfig.ActsPixelEstimatedTrackParamsAnalysisAlgCfg" \
    --preExec "flags.Tracking.doITkFastTracking=True; \
               flags.Tracking.doTruth=False; \
               flags.DQ.useTrigger=False; \
               flags.Acts.doAnalysis=True; \
               flags.Output.HISTFileName=\"ActsMonitoringOutput.root\";" \
    --perfmon fullmonmt \
    --maxEvents 5

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

echo "download latest result..."
art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
ls -la "$lastref_dir"

# Needs to be updated!!!
run "dcube-last" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_shifter_last \
    -c ${dcubeXmlAbsPath} \
    -r ${lastref_dir}/ActsMonitoringOutput.root \
    ActsMonitoringOutput.root
