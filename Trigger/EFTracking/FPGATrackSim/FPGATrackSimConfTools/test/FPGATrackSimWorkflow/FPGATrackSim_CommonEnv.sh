GEO_TAG="ATLAS-P2-RUN4-03-00-00"
RDO_SINGLE_MUON="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/RDO/reg0_singlemu.root"
RDO_TTBAR="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1"
RDO_EVT=500 # used for map/bank generation

# instructions on how to change version of files can be found in https://twiki.cern.ch/twiki/bin/view/Atlas/EFTrackingSoftware
MAP_9L_VERSION="v0.22"
MAP_5L_VERSION="v0.22"
MAP_9L_GNN_VERSION="v0.10"

BANK_9L_VERSION="v0.20"
BANK_5L_VERSION="v0.21"

export CALIBPATH=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/:$CALIBPATH

MAPS_9L="maps_9L/OtherFPGAPipelines/${MAP_9L_VERSION}/"
MAPS_5L="maps_5L/InsideOut/${MAP_5L_VERSION}/"
MAPS_9L_GNN="maps_9L/GNN/${MAP_9L_GNN_VERSION}/"

BANKS_9L="banks_9L/${BANK_9L_VERSION}/"
BANKS_5L="banks_5L/${BANK_5L_VERSION}/"

COMBINED_MATRIX="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/${BANKS_5L}/combined_matrix_reg34.root"

ONNX_INPUT_FAKE="${BANKS_5L}HT_class_v7_longer_training_pruned_0.5.onnx"
ONNX_INPUT_PARAM="${BANKS_5L}HT_param_v7_long_training_400_epochs_pruned_0.5.onnx"
ONNX_INPUT_HIT="${BANKS_5L}NNPathfinderHit_4Hits_V005_200e.onnx"
ONNX_INPUT_VOL="${BANKS_5L}NNPathfinderVol_4Hits_V005_200e.onnx"

GNN_MODULE_MAP="GNN/v0.10/FPGATrackSim_DoubletModuleMap_v1.root" # New training will be done later
GNN_ONNX_MODEL="GNN/v0.10/edge_classifier-InteractionGNN2-v1.onnx" # New training will be done later
GNN_METRIC_LEARNING="GNN/v0.10/graph_construction-MetricLearning-v2.onnx" # New training will be done in the future

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
        -i|--inputFile)
            RDO_ANALYSIS="$2"
            shift 2 ;;
        -n|--events)
            RDO_EVT_ANALYSIS="$2";
            if [ "$RDO_EVT_ANALYSIS" -gt 10 ] || [ "$RDO_EVT_ANALYSIS" -eq -1 ]; then
                RUN_CKF=False
            fi
            shift 2 ;;
        -s|--skip-events) SKIP_EVENTS="$2"; shift 2 ;;
        -q|--noDataOutput) WRITE_UPSTREAM_OUTPUT_DATA=False; shift ;;
        --) shift; break ;;
        *) echo "Unknown option: $1"; return 1 ;;
    esac
done

RDO_ANALYSIS="${RDO_ANALYSIS// /, }"

# Print final configuration
echo "Configuration:"
echo "  RDO File(s) = $RDO_ANALYSIS"
echo "  sampleType = $SAMPLE_TYPE"
echo "  Events to run = $RDO_EVT_ANALYSIS"
echo "  Events to skip = $SKIP_EVENTS"
echo "  Run CKF = $RUN_CKF"
echo "  WRITE_OUTPUT_DATA = $WRITE_UPSTREAM_OUTPUT_DATA"
