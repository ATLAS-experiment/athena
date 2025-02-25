GEO_TAG="ATLAS-P2-RUN4-03-00-00"
RDO_SINGLE_MUON="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/RDO/reg0_singlemu.root"
RDO_TTBAR="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1"
RDO_EVT=500 # used for map/bank generation

# instructions on how to change version of files can be found in https://twiki.cern.ch/twiki/bin/view/Atlas/EFTrackingSoftware
MAP_9L_VERSION="v0.22"
MAP_5L_VERSION="v0.12"
MAP_9L_GNN_VERSION="v0.10"

BANK_9L_VERSION="v0.20"
BANK_5L_VERSION="v0.11"

export CALIBPATH=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/:$CALIBPATH
MAPS_9L="maps_9L/OtherFPGAPipelines/${MAP_9L_VERSION}/"
MAPS_5L="maps_5L/InsideOut/${MAP_5L_VERSION}/"
MAPS_9L_GNN="maps_9L/GNN/${MAP_9L_GNN_VERSION}/"

BANKS_9L="banks_9L/${BANK_9L_VERSION}/"
BANKS_5L="banks_5L/${BANK_5L_VERSION}/"

COMBINED_MATRIX="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/${BANKS_9L}/combined_matrix.root"

ONNX_INPUT_FAKE="${BANKS_9L}ClassificationHT_v5.onnx"
ONNX_INPUT_PARAM="${BANKS_9L}ParamEstimationHT_v5.onnx"
ONNX_INPUT_HIT="${BANKS_9L}Ath_Extrap_v51_6_superBig_0_outsideIN.onnx"
ONNX_INPUT_VOL="${BANKS_9L}HT_detector_v6_3.onnx"

GNN_MODULE_MAP="GNN/v0.10/FPGATrackSim_DoubletModuleMap_v1.root" # New training will be done later
GNN_ONNX_MODEL="GNN/v0.10/edge_classifier-InteractionGNN2-v1.onnx" # New training will be done later

# set default values
RUN_CKF=True
RDO_EVT_ANALYSIS=-1
SKIP_EVENTS=0
RDO_ANALYSIS=$RDO_SINGLE_MUON
SAMPLE_TYPE='singleMuons'
WRITE_UPSTREAM_OUTPUT_DATA=True

# arg parser
while [[ $# -gt 0 ]]; do
    case "$1" in
        -t|--ttbar) 
            SAMPLE_TYPE='skipTruth'
            RDO_ANALYSIS=$RDO_TTBAR
            RUN_CKF=False
            shift ;;
        -m|--single-muon)
            SAMPLE_TYPE='singleMuons'
            RDO_ANALYSIS=$RDO_SINGLE_MUON
            RUN_CKF=True
            shift ;;
        -n|--events) RDO_EVT_ANALYSIS="$2"; shift 2 ;;
        -s|--skip-events) SKIP_EVENTS="$2"; shift 2 ;;
        -q|--noDataOutput) WRITE_UPSTREAM_OUTPUT_DATA=False; shift ;;
        --) shift; break ;;
        *) echo "Unknown option: $1"; return 1 ;;
    esac
done

# Print final configuration
echo "Configuration:"
echo "  RDO_ANALYSIS = $RDO_ANALYSIS"
echo "  SAMPLE_TYPE = $SAMPLE_TYPE"
echo "  RDO_EVT_ANALYSIS = $RDO_EVT_ANALYSIS"
echo "  SKIP_EVENTS = $SKIP_EVENTS"
echo "  RUN_CKF = $RUN_CKF"
echo "  WRITE_OUTPUT_DATA = $WRITE_UPSTREAM_OUTPUT_DATA"