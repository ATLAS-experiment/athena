GEO_TAG="ATLAS-P2-RUN4-03-00-00"
RDO_SINGLE_MUON="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/RDO/reg0_singlemu.root"
RDO_TTBAR="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1"
RDO_EVT=500 # used for map/bank generation
# instructions on how to change version of files can be found in https://twiki.cern.ch/twiki/bin/view/Atlas/EFTrackingSoftware
MAP_9L_VERSION="v0.22"
MAP_5L_VERSION="v0.12"

BANK_9L_VERSION="v0.20"
BANK_5L_VERSION="v0.11"

export CALIBPATH=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/:$CALIBPATH
MAPS_9L="maps_9L/OtherFPGAPipelines/${MAP_9L_VERSION}/"
MAPS_5L="maps_5L/InsideOut/${MAP_5L_VERSION}/"

BANKS_9L="banks_9L/${BANK_9L_VERSION}/"
BANKS_5L="banks_5L/${BANK_5L_VERSION}/"

COMBINED_MATRIX="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/${BANKS_9L}/combined_matrix.root"

ONNX_INPUT_FAKE="${BANKS_9L}ClassificationHT_v5.onnx"
ONNX_INPUT_PARAM="${BANKS_9L}ParamEstimationHT_v5.onnx"

RUN_CKF=True
RUN_ON_TTBAR=False

while getopts 't' opt; do
  case $opt in
    t) RUN_ON_TTBAR=True;;
  esac
done

if [ -z $ArtJobType ]; then # if not an ART, run for a few ttbar events (proved useful to catch edge cases at region boundaries)
    RDO_ANALYSIS=$RDO_TTBAR
    RDO_EVT_ANALYSIS=1
    SAMPLE_TYPE='skipTruth'
elif [ "$RUN_ON_TTBAR" = True ]; then # code will run on ttbar events for ART
    RDO_ANALYSIS=$RDO_TTBAR
    RDO_EVT_ANALYSIS=-1
    SAMPLE_TYPE='skipTruth'
else
    RDO_ANALYSIS=$RDO_SINGLE_MUON
    RDO_EVT_ANALYSIS=-1
    SAMPLE_TYPE='singleMuons'
fi

if [ "$SAMPLE_TYPE" == "skipTruth" ]; then # in case of ttbar disable CKF (Related to EFTRACK-591)
    echo "Turning off CKF..."
    RUN_CKF=False
fi

