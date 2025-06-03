#!/bin/bash
set -e

echo "Generate a small wrapper file"
FPGATrackSimWrapperFileGeneration.sh
echo "Generating wrapper file is done"

echo "Run F-600 analysis on the wrapper file"
FPGATrackSimAnalysisOnWrapper.sh
echo "Running on wrapper file is done..."

echo "Generate maps from wrapper file"
FPGATrackSimMapGeneration.sh
echo "Generating maps is done"

echo "Generate banks from maps"
FPGATrackSimBankGeneration.sh
echo "Generating banks is done"

echo "Testing LayerStudy"
FPGATrackSimLayerStudy.sh -n 10
echo "LayerStudy test is done"

# # Common variables
OUTPUT_AOD_FILE="FPGATrackSim_CI_AOD.root"

echo "Running FPGATrackSim F-410 for a few single-mu events"
FPGATrackSim_F410.sh -m -n 45 -c -o $OUTPUT_AOD_FILE
echo "validating output AOD from F-410"
python -m FPGATrackSimConfTools.FPGATrackSimValidateAODOutput $OUTPUT_AOD_FILE

echo "Running FPGATrackSim F-610 for a few single-mu events"
FPGATrackSim_F610.sh -m -n 45 -c -o $OUTPUT_AOD_FILE
echo "validating output AOD from F-610"
python -m FPGATrackSimConfTools.FPGATrackSimValidateAODOutput $OUTPUT_AOD_FILE

echo "Running F-100 for a few ttbar events"
FPGATrackSim_F100.sh -t -n 1 -c -o $OUTPUT_AOD_FILE
echo "validating output AOD from F-100"
python -m FPGATrackSimConfTools.FPGATrackSimValidateAODOutput $OUTPUT_AOD_FILE

echo "Checking if IDTPM can run on the output AOD file"
get_files -data FPGATrackSimConfTools/IDTPM_ttbar_allRegions.json
runIDTPM.py --inputFileNames=$OUTPUT_AOD_FILE \
                --outputFilePrefix="IDTPM.CI_TEST" \
                --trkAnaCfgFile="FPGATrackSimConfTools/IDTPM_ttbar_allRegions.json"