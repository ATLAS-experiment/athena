if [ -z "$1" ]; then
    echo "Error: No argument provided. Please provide the output AOD file including clusters from the pipeline."
    exit 1
fi

INPUT_AOD_FILE=$1
PREFIX="DataPrep"
ATHENA_SOURCE="${ATLAS_RELEASE_BASE}/Athena/${Athena_VERSION}/InstallArea/${Athena_PLATFORM}/src/"
IDTPM_CONFIG="${ATHENA_SOURCE}/Trigger/EFTracking/EFTrackingFPGAIntegration/script/IDTPM_allRegions.json"

runIDTPM.py --inputFileNames=$INPUT_AOD_FILE \
                --outputFilePrefix="IDTPM.${PREFIX}" \
                --trkAnaCfgFile=$IDTPM_CONFIG
