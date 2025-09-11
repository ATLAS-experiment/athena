GEO_TAG="ATLAS-P2-RUN4-03-00-00"
RDO_SINGLE_MUON="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/ATLAS-P2-RUN4-03-00-00/RDO/rdo_singleMu_alleta.root"
RDO_TTBAR="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1"
WRAPPER="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/Wrappers/v0.11/FPGATrackSimWrapper.root"
RDO_EVT=50 # used for map/bank generation

# instructions on how to change version of files can be found in https://twiki.cern.ch/twiki/bin/view/Atlas/EFTrackingSoftware
MAP_9L_VERSION="v0.23"
MAP_5L_VERSION="v0.33"
MAP_9L_GNN_VERSION="v0.10"

BANK_9L_VERSION="v0.20"
BANK_5L_VERSION="v0.33"

NN_1ST="v0.11"
NN_2ND="v0.11"

export CALIBPATH=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/:$CALIBPATH

MAPS_9L="maps_9L/OtherFPGAPipelines/${MAP_9L_VERSION}/"
MAPS_5L="maps_5L/InsideOut/${MAP_5L_VERSION}/"
MAPS_9L_GNN="maps_9L/GNN/${MAP_9L_GNN_VERSION}/"

BANKS_9L="banks_9L/${BANK_9L_VERSION}/"
BANKS_5L="banks_5L/${BANK_5L_VERSION}/"

COMBINED_MATRIX="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/${BANKS_5L}/combined_matrix_reg34.root"

# NN 1st stage
ONNX_INPUT_FAKE="NN/1stStage/${NN_1ST}/Class_5Hits_V007"
ONNX_INPUT_PARAM="NN/1stStage/${NN_1ST}/Param_5Hits_V004"
# NN 2nd stage
ONNX_INPUT_PARAM_2ND="NN/2ndStage/${NN_2ND}/Param_13Hits_V007"
ONNX_INPUT_FAKE_2ND="NN/2ndStage/${NN_2ND}/Class_13Hits_V007"
ONNX_INPUT_HIT="NN/2ndStage/${NN_2ND}/NNPathfinderHit_4Hits_V011"
ONNX_INPUT_VOL="NN/2ndStage/${NN_2ND}/NNPathfinderVol_4Hits_V011"

# GNN
GNN_METRIC_LEARNING="GNN/v0.12/graph_construction-MetricLearning-FPGATrackSim.onnx"
GNN_MODULE_MAP="GNN/v0.12/FPGATrackSim_DoubletModuleMap_final"
GNN_ONNX_MODEL="GNN/v0.12/edge_classifier-InteractionGNN2-FPGATrackSim_final"

# set default values
RUN_CKF=True
RDO_EVT_ANALYSIS=-1
SKIP_EVENTS=0
RDO_ANALYSIS=$RDO_SINGLE_MUON
SAMPLE_TYPE='singleMuons'
WRITE_UPSTREAM_OUTPUT_DATA=True
WRITE_XAOD_CLUSTERS=False

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
        -c|--writeClusters)
            WRITE_XAOD_CLUSTERS=True
            shift ;;
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
