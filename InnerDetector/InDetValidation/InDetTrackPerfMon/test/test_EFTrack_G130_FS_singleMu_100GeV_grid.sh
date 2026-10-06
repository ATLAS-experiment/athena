#!/bin/bash
# art-description: Nightly test to compare G-130 vs C-230 (Full-scan) for EFTrack studies using singleMu 100GeV sample
# art-type: grid
# art-include: main/Athena/x86_64-el9-gcc15-opt
# art-architecture: {"gpu_spec": {"vendor": "nvidia", "version": ">=13.3", "model": {"pattern": ".*(P100|V100).*", "excl": true}}}
# art-input: mc21_14TeV:mc21_14TeV.900498.PG_single_muonpm_Pt100_etaFlatnp0_43.recon.RDO.e8557_s4422_r16128
# art-input-nfiles: 400
# art-output: IDTPM.*.root
# art-output: *.json
# art-output: *.xml
# art-output: *.html
# art-output: *.log
# art-output: dcube*
# art-html: dcube_cmp

## Input parameters
pipelineName='G130'
SampleName='singleMu_100GeV'
OutSampleName="${pipelineName}_FS.${SampleName}"
TrkCollName='InDetTrackParticles'
TrkSeedCollName='SiSPSeedSegmentsActsPixelTrackParticles'
referencePath='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetTrackPerfMon/EFTrackRefereceHistograms/'
referenceName="C230_FS.${SampleName}"
referenceName_absPath="${referencePath}/IDTPM.${referenceName}.HIST.root"
refLabel="C-230"
testLabel="G-130"

## search in $DATAPATH for matching files
IDTPMjsonConfig='EFTrack_singleMu_FS_IDTPMconfig.json'
dcubeXmlIDTPMconfig='dcube_EFTrack_SingleMu.xml'

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
    ## if _diffOK is in name, then we expect dcube differences, so don't flag as an error
    if [[ $rc == 1 && "${name}" =~ "_diffOK" ]]; then
      rc=0
    fi
    rm step_${name}.sh
    echo "art-result: $rc ${name}"
    ## if _skipRC is in name skip exit condition
    if [[ "${name}" =~ "_skipRC" ]]; then
      return 0
    fi
    # don't exit only for ERRORs detected in reco logfile (rc=68)
    if [ $rc != 0 -a $rc != 68 ]; then
        exit $rc
    fi
    return $rc
}

## Getting the comma-separated list of input RDOs
## Prefer ArtInFile in case of grid ART (where it should be available)
if [ -n "${ArtInFile}" ]; then
    # ArtInFile is space-separated; convert to comma-separated for runReco
    echo "ArtInFile: ${ArtInFile}"
    InputRDOfiles="${ArtInFile// /,}"
    echo "InputRDOfiles: ${InputRDOfiles}"
else # otherwise fall back to getEFTrackSample.py
    echo "ArtInFile not set, falling back to getEFTrackSample.py..."
    InputRDOfiles=$( getEFTrackSample.py -s ${SampleName} )
    if [ ! -f "${InputRDOfiles}" ]; then
        echo "art-result: 1 Sample ${SampleName} not found"
        exit 1
    fi
fi

## Track reconstruction step. See runReco_G130_FS.sh --help for list of supported options.
run "${pipelineName}" \
  runReco_G130_FS.sh \
    -i "${InputRDOfiles}" \
    -o "${OutSampleName}.AOD.pool.root" \
    -n -1 \
    "$@"

## Don't run if IDTPM json config is not found
if [ ! -f "$IDTPMjsonConfig_absPath" ]; then
    echo "art-result: 1 $IDTPMjsonConfig_absPath not found"
    exit 1
fi

## Copying json config in the output directory
echo "Running IDTPM with the following json config:"
## change the name of the track collection to monitor and copy json config in work dir
jq --arg coll "$TrkCollName" --arg seedcoll "$TrkSeedCollName" '
  (.TruthMuons.OfflineTrkKey         = $coll) |
  (.TruthMuons_EFsel.OfflineTrkKey   = $coll) |
  (.TrackSeeds.OfflineTrkKey         = $seedcoll) |
  (.TrackSeeds.enabled               = true)
' "$IDTPMjsonConfig_absPath" | tee ${cwd}/IDTPMconfig.json

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
run "dcube_diffOK" \
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
    -r ${lastref_dir}/IDTPM.${OutSampleName}.HIST.root \
    -R "last_nightly" -T "new_nightly" \
    -o "TrkAnaSummary_last_\&TrkAnaName\&.html" \
    -a "${allTrkAna}"
