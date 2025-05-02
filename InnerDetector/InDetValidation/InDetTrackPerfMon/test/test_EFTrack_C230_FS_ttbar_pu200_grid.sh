#!/bin/bash
# art-description: Nightly test to compare C-230 vs C-000 (Full-scan) for EFTrack studies using ttbar pu200 sample
# art-type: grid
# art-include: main/Athena
# art-output: IDTPM.*.root
# art-output: *.json
# art-output: *.xml
# art-output: *.html
# art-output: *.log
# art-output: dcube*
# art-html: dcube_cmp


## Input parameters
pipelineName='C230'
SampleName='ttbar_pu200'  # as defined in samplesDict of InDetTrackPerfMon/scripts/getEFTrackSample.py
OutSampleName="${pipelineName}_FS.${SampleName}"
TrkCollName='InDetTrackParticles'
referencePath='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetTrackPerfMon/EFTrackRefereceHistograms/'
referenceName="C000_FS.${SampleName}"
referenceName_absPath="${referencePath}/IDTPM.${referenceName}.HIST.root"
refLabel="C-000"
testLabel="C-230"

## search in $DATAPATH for matching files
IDTPMjsonConfig='EFTrack_base_FS_noDoubleRatio_IDTPMconfig.json'
dcubeXmlIDTPMconfig='dcube_config_EFTrack_base_FS_noDoubleRatio.xml'

IDTPMjsonConfig_absPath=$( find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 2 -name $IDTPMjsonConfig -print -quit 2>/dev/null )
dcubeXmlIDTPMconfig_absPath=$( find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 2 -name $dcubeXmlIDTPMconfig -print -quit 2>/dev/null )
cwd=$(pwd)

run () {
    name="${1}"
    cmd="${@:2}"
    echo -e "\n\n--------------\nRunning ${name}..."
    echo -e "\n---> ${name}" >> "${cwd}/commands.log"
    echo "${cmd}" >> "${cwd}/commands.log"
    echo "#!/bin/bash" >> step_${name}.sh
    echo "${cmd} &> step_${name}.log" >> step_${name}.sh
    chmod 777 step_${name}.sh
    time $(pwd)/step_${name}.sh
    rc=$?
    rm step_${name}.sh
    echo "art-result: $rc ${name}"
    ## if _skipRC is in name skip exit condition
    if [[ "${name}" =~ "_skipRC" ]]; then
      return 0
    fi
    if [ $rc != 0 ]; then
        exit $rc
    fi
    return $rc
}

## Getting the comma-separated list of input RDOs
InputRDOfiles=$( getEFTrackSample.py -s ${SampleName} )
if [ ! -f "${InputRDOfiles}" ]; then
    echo "art-result: 1 Sample ${SampleName} not found"
    exit 1
fi

## Track reconstruction step
run "${pipelineName}" \
  runReco_C230_FS.sh \
    -i ${InputRDOfiles} \
    -o "${OutSampleName}.AOD.pool.root"
    #-n 10

## Don't run if IDTPM json config is not found
if [ ! -f "$IDTPMjsonConfig_absPath" ]; then
    echo "art-result: 1 $IDTPMjsonConfig_absPath not found"
    exit 1
fi

## Copying json config in the output directory
echo "Running IDTPM with the following json config:"
## change the name of the track collection to monitor and copy json config in work dir
cat $IDTPMjsonConfig_absPath | sed "s|_TRKCOLLNAME_|${TrkCollName}|g" | tee ${cwd}/IDTPMconfig.json

## IDTPM step
run "IDTPM" \
  runIDTPM.py \
    --inputFileNames "${OutSampleName}.AOD.pool.root" \
    --outputFilePrefix "IDTPM.${OutSampleName}" \
    --trkAnaCfgFile "${cwd}/IDTPMconfig.json"

## Don't run if dcube xml config is not found
if [ ! -f "$dcubeXmlIDTPMconfig_absPath" ]; then
    echo "art-result: 1 $dcubeXmlIDTPMconfig_absPath not found"
    exit 1
fi

## Don't run if reference histogram file is not found
if [ ! -f "$referenceName_absPath" ]; then
    echo "art-result: 1 $referenceName_absPath not found"
    exit 1
fi

## dcube step
run "dcube_skipRC" \
  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_cmp \
    -c ${dcubeXmlIDTPMconfig_absPath} \
    -r ${referenceName_absPath} \
    -R "ref=${refLabel}" -M "mon=${testLabel}" \
    IDTPM.${OutSampleName}.HIST.root

## reading json keys from IDTPM config
allTrkAna=""
for key in $( jq 'keys | .[]' ${cwd}/IDTPMconfig.json ); do
  if [ "x${allTrkAna}" == "x" ]; then
    allTrkAna="$( echo $key | sed 's|\"||g' )"
  else
    allTrkAna="${allTrkAna},$( echo $key | sed 's|\"||g' )"
  fi
done

## Printing summary
run "PrintSummaryTable_skipRC" \
  PrintTrkAnaSummary.py \
    -t IDTPM.${OutSampleName}.HIST.root \
    -r ${referenceName_absPath} \
    -R "${refLabel}" -T "${testLabel}" \
    -a "${allTrkAna}"

## Now monitor vs the last ART results
echo "download latest result..."
lastref_dir=last_results
art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
ls -la "$lastref_dir"

## dcube last step
run "dcube_last_skipRC" \
  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_last \
    -c ${dcubeXmlIDTPMconfig_absPath} \
    -r ${lastref_dir}/IDTPM.${OutSampleName}.HIST.root \
    IDTPM.${OutSampleName}.HIST.root

## Printing last summary
run "PrintSummaryTable_last_skipRC" \
  PrintTrkAnaSummary.py \
    -t IDTPM.${OutSampleName}.HIST.root \
    -r ${referenceName_absPath} \
    -R "last_nightly" -T "new_nightly" \
    -o "TrkAnaSummary_last_&TrkAnaName&.html" \
    -a "${allTrkAna}"
