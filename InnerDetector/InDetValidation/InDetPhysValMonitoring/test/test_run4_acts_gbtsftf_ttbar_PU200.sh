#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction with ACTS and GBTS seeding, PU 200
# art-type: grid
# art-include: main/Athena
# art-output: acts-*.root
# art-output: idpvm*.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_gbts_last
# art-athena-mt: 8

lastref_dir=last_results
dcubeXml=dcube_IDPVMPlots_ACTS_CKF_ITk_techeff.xml
n_events=-1
rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")

conditionsTag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

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
    # Only report hard failures for comparison GBTS-ACTS since we know
    # they are different. We do not expect this test to succeed
    [ "${name}" = "dcube-gbts-acts" ] && [ $rc -ne 255 ] && rc=0
    echo "art-result: $rc ${name}"
    return $rc
}

export ATHENA_CORE_NUMBER=4

# Run Athena with ACTS fast tracking and FTF GBTS seeding
run "Reconstruction-gbts" \
    Reco_tf.py \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" \
    --preExec "from ActsConfig.ActsConfigFlags import SeedingStrategy; \
               flags.Acts.SeedingStrategy=SeedingStrategy.GbtsFtf; \
               flags.Tracking.writeExtendedSi_PRDInfo=True; \
               flags.Acts.doMonitoring=True; \
               flags.Acts.doAnalysis=True; \
               flags.Acts.doAnalysisNtuples=False; \
               flags.DQ.useTrigger=False; \
               flags.Output.HISTFileName='acts-analysis.gbts.root'" \
    --conditionsTag "default:${conditionsTag}" \
    --inputRDOFile ${rdo} \
    --outputAODFile AOD.gbts.root \
    --perfmon fullmonmt \
    --maxEvents ${n_events} \
    --multithreaded

reco_rc=$?

mv log.RAWtoALL log.RAWtoALL.GBTS
mv acts-expert-monitoring.root acts-expert-monitoring.gbts.root

if [ $reco_rc != 0 -a $reco_rc != 68 ]; then
    exit $reco_rc
fi

run "IDPVM-gbts" \
    runIDPVM.py \
    --filesInput AOD.gbts.root \
    --outputFile idpvm.gbts.root \
    --doHitLevelPlots \
    --HSFlag All \
    --doTechnicalEfficiency \
    --doExpertPlots \
    --OnlyTrackingPreInclude

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

# Run Athena with ACTS fast tracking and triplet seeding
run "Reconstruction-acts" \
    Reco_tf.py \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" \
    --preExec "from ActsConfig.ActsConfigFlags import SeedingStrategy; \
               flags.Acts.SeedingStrategy=SeedingStrategy.GridTriplet; \
	       flags.Tracking.writeExtendedSi_PRDInfo=True; \
               flags.Acts.doMonitoring=True; \
               flags.Acts.doAnalysis=True; \
               flags.Acts.doAnalysisNtuples=False; \
               flags.DQ.useTrigger=False; \
               flags.Output.HISTFileName='acts-analysis.acts.root'" \
    --conditionsTag "default:${conditionsTag}" \
    --ignorePatterns "${ignore_pattern}" \
    --inputRDOFile ${rdo} \
    --outputAODFile AOD.acts.root \
    --perfmon fullmonmt \
    --maxEvents ${n_events} \
    --multithreaded

reco_rc=$?

mv log.RAWtoALL log.RAWtoALL.ACTS
mv acts-expert-monitoring.root acts-expert-monitoring.acts.root

if [ $reco_rc = 0 -o $reco_rc = 68 ]; then
  run "IDPVM-acts" \
      runIDPVM.py \
      --filesInput AOD.acts.root \
      --outputFile idpvm.acts.root \
      --doTightPrimary \
      --doHitLevelPlots \
      --HSFlag All \
      --doTechnicalEfficiency \
      --doExpertPlots \
      --OnlyTrackingPreInclude
fi

echo "download latest result..."
art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
ls -la "$lastref_dir"

run "dcube-gbts-last" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_gbts_last \
    -c ${dcubeXmlAbsPath} \
    -r ${lastref_dir}/idpvm.gbts.root \
    idpvm.gbts.root

if [ $reco_rc = 0 -o $reco_rc = 68 ]; then
  # Compare performance WRT default seeding
  run "dcube-gbts-acts" \
      $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
      -p -x dcube_gbts_acts \
      -c ${dcubeXmlAbsPath} \
      -r idpvm.acts.root \
      -M "gbts" \
      -R "acts" \
      idpvm.gbts.root
fi

echo "Clean up output directory (based on compiler)"
clean_up_outdir.sh ${AtlasBuildBranch} ${AtlasProject} ${AtlasBuildStamp}
