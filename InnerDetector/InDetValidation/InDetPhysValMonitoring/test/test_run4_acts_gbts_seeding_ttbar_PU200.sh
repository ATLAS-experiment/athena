#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction with ACTS and GBTS seeding, PU 200
# art-type: grid
# art-include: main/Athena
# art-output: acts-*.root
# art-output: idpvm*.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_gbts_shifter_last
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
    [ "${name}" = "dcube-gbtsacts-gbtsftf" ] && [ $rc -ne 255 ] && rc=0
    echo "art-result: $rc ${name}"
    return $rc
}

export ATHENA_CORE_NUMBER=8

# Run Athena with ACTS fast tracking and GBTS core seeding
run "Reconstruction-gbtsacts" \
    Reco_tf.py \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsWorkflowFlags" \
    --preExec "from ActsConfig.ActsConfigFlags import SeedingStrategy; \
               flags.Acts.SeedingStrategy=SeedingStrategy.Gbts; \
               flags.Tracking.doPixelDigitalClustering=True; \
               flags.Tracking.writeExtendedSi_PRDInfo=True; \
               flags.Acts.doMonitoring=True; \
               flags.Acts.doAnalysis=True; \
               flags.Acts.doAnalysisNtuples=False; \
               flags.DQ.useTrigger=False; \
               flags.Output.HISTFileName='acts-analysis.gbtsacts.root'" \
    --conditionsTag "default:${conditionsTag}" \
    --inputRDOFile ${rdo} \
    --outputAODFile AOD.gbtsacts.pool.root \
    --perfmon fullmonmt \
    --maxEvents ${n_events} \
    --multithreaded


reco_rc=$?

mv log.RAWtoALL log.RAWtoALL.gbtsacts
mv acts-expert-monitoring.root acts-expert-monitoring.gbtsacts.root

if [ $reco_rc != 0 -a $reco_rc != 68 ]; then
    exit $reco_rc
fi

run "IDPVM-gbtsacts" \
    runIDPVM.py \
    --filesInput AOD.gbtsacts.pool.root \
    --outputFile idpvm.gbtsacts.root \
    --doHitLevelPlots \
    --HSFlag All \
    --doTechnicalEfficiency \
    --doExpertPlots \
    --OnlyTrackingPreInclude

reco_rc=$?
if [  $reco_rc != 0 -a $reco_rc != 68 ]; then
    exit $reco_rc
fi
# Run Athena with ACTS fast tracking and FTF GBTS seeding
run "Reconstruction-gbtsftf" \
    Reco_tf.py \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsWorkflowFlags" \
    --preExec "from ActsConfig.ActsConfigFlags import SeedingStrategy; \
               flags.Acts.SeedingStrategy=SeedingStrategy.GbtsFtf; \
               flags.Tracking.doPixelDigitalClustering=True; \
               flags.Tracking.writeExtendedSi_PRDInfo=True; \
               flags.Acts.doMonitoring=True; \
               flags.Acts.doAnalysis=True; \
               flags.Acts.doAnalysisNtuples=False; \
               flags.DQ.useTrigger=False; \
               flags.Output.HISTFileName='acts-analysis.gbtsftf.root'" \
    --conditionsTag "default:${conditionsTag}" \
    --inputRDOFile ${rdo} \
    --outputAODFile AOD.gbtsftf.pool.root \
    --perfmon fullmonmt \
    --maxEvents ${n_events} \
    --multithreaded

reco_rc=$?

mv log.RAWtoALL log.RAWtoALL.gbtsftf
mv acts-expert-monitoring.root acts-expert-monitoring.gbtsftf.root

if [ $reco_rc != 0 -a $reco_rc != 68 ]; then
    exit $reco_rc
fi

run "IDPVM-gbtsftf" \
    runIDPVM.py \
    --filesInput AOD.gbtsftf.pool.root \
    --outputFile idpvm.gbtsftf.root \
    --doHitLevelPlots \
    --HSFlag All \
    --doTechnicalEfficiency \
    --doExpertPlots \
    --OnlyTrackingPreInclude

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

echo "download latest result..."
art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
mv "${lastref_dir}/idpvm.gbts.root" "${lastref_dir}/idpvm.gbtsacts.root"
ls -la "$lastref_dir"

run "dcube-gbtsacts-last" \
    "$ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py" \
    -p -x dcube_gbts_shifter_last \
    -c "${dcubeXmlAbsPath}" \
    -r "${lastref_dir}/idpvm.gbtsacts.root" \
    idpvm.gbtsacts.root

if [ $reco_rc = 0 -o $reco_rc = 68 ]; then
  # Compare ACTS and FTF
  run "dcube-gbtsacts-gbtsftf" \
      "$ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py" \
      -p -x dcube_gbtsacts_gbtsftf \
      -c "${dcubeXmlAbsPath}" \
      -r idpvm.gbtsftf.root \
      -M "GbtsActs" \
      -R "GbtsFtf" \
      idpvm.gbtsacts.root
fi
