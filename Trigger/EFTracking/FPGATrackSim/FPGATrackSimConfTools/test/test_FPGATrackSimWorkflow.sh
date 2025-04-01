#!/bin/bash
# art-description: Test running FPGA sim workflow
# art-type: grid
# art-include: main/Athena
# art-memory: 8192
# art-output: *.txt
# art-output: *.root
set -e

source FPGATrackSim_CommonEnv.sh

echo "Running over ${RDO_EVT} events"

# make wrapper file
echo "... RDO to AOD with sim"
Reco_tf.py --CA \
    --steering doRAWtoALL \
    --preExec "flags.Trigger.FPGATrackSim.wrapperFileName='wrapper.root'" \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateTracksFlags" \
    --postInclude "FPGATrackSimSGInput.FPGATrackSimSGInputConfig.FPGATrackSimSGInputCfg" \
    --inputRDOFile "${RDO_SINGLE_MUON}" \
    --outputAODFile AOD.pool.root \
    --maxEvents -1
ls -l
echo "... RDO to AOD with sim, this part is done ..."

# generate maps
echo "... Maps Making"
python -m FPGATrackSimConfTools.FPGATrackSimMapMakerConfig \
    --filesInput=wrapper.root \
    OutFileName="MyMaps_" \
    doInsideOut=True \
    nSlices=1 \
    Trigger.FPGATrackSim.region=34 \
    Trigger.FPGATrackSim.spacePoints=False \
    Trigger.FPGATrackSim.Hough.secondStage=False \
    GeoModel.AtlasVersion="${GEO_TAG}"
ls -l
echo "... Maps Making, this part is done ..."

mkdir -p maps
mv MyMaps_region*.rmap maps/region34.rmap
mv MyMaps_region*.subrmap maps/region34.subrmap
mv MyMaps_region*.pmap maps/region34.pmap
mv MyMaps_region*.patt maps/region34.patt
mv *radii.txt maps/region34_radii.txt
cp /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/${MAPS_5L}/region34_lyrmap.json maps/region34_lyrmap.json
cp /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/${GEO_TAG}/${MAPS_5L}/*.py maps/
touch maps/moduleidmap

echo "... Banks generation"
python -m FPGATrackSimBankGen.FPGATrackSimBankGenConfig \
    --filesInput="${RDO_SINGLE_MUON}" \
    --evtMax=-1 \
    Trigger.FPGATrackSim.Hough.genScan=True \
    Trigger.FPGATrackSim.GenScan.noCuts=False \
    Trigger.FPGATrackSim.Hough.secondStage=False \
    Trigger.FPGATrackSim.mapsDir="maps/"
ls -l
echo "... Banks generation, this part is done ..."

echo "... const generation on combined matrix file"
python -m FPGATrackSimBankGen.FPGATrackSimBankConstGenConfig \
    Trigger.FPGATrackSim.FPGATrackSimMatrixFileRegEx="${COMBINED_MATRIX}" \
    Trigger.FPGATrackSim.Hough.genScan=True \
    Trigger.FPGATrackSim.Hough.secondStage=False \
    Trigger.FPGATrackSim.mapsDir="maps/" \
    --evtMax=1
ls -l
echo "... const generation on combined matrix file, this part is done ..."

mkdir -p banks 
mv sectors* corr* banks

echo "... analysis on wrapper"
python -m FPGATrackSimConfTools.FPGATrackSimAnalysisConfig \
    Trigger.FPGATrackSim.wrapperFileName="wrapper.root" \
    Trigger.FPGATrackSim.mapsDir=./maps/ \
    Trigger.FPGATrackSim.pipeline='F-600' \
    Trigger.FPGATrackSim.tracking=True \
    Trigger.FPGATrackSim.Hough.secondStage=False \
    Trigger.FPGATrackSim.bankDir=./banks/
ls -l
echo "... analysis on wrapper, this part is done ..."

echo "... analysis output verification"
cat << EOF > checkHist.C
{
    _file0->cd("FPGATrackSimLogicalHitsProcessAlg");
    TH1* h = (TH1*)gDirectory->Get("nroads_1st");
    if ( h == nullptr )
        throw std::runtime_error("oh dear, after all of this there is no roads histogram");
    h->Print(); 
    if ( h->GetEntries() == 0 ) {
        throw std::runtime_error("oh dear, after all of this there are zero roads");
    }
}
EOF

root -b -q monitoring.root checkHist.C
rm monitoring.root
echo "... wrapper analysis output verification, this part is done ..."


echo "... analysis on RDO"
python -m FPGATrackSimConfTools.FPGATrackSimAnalysisConfig \
        --evtMax=${RDO_EVT} \
        --filesInput=${RDO_SINGLE_MUON} \
        Trigger.FPGATrackSim.mapsDir=./maps/ \
        Trigger.FPGATrackSim.bankDir=./banks/ \
        Trigger.FPGATrackSim.pipeline='F-600' \
        Trigger.FPGATrackSim.tracking=True \
        Trigger.FPGATrackSim.sampleType=${SAMPLE_TYPE} \
        Trigger.FPGATrackSim.doEDMConversion=True \
        Trigger.FPGATrackSim.doOverlapRemoval=True \
        Trigger.FPGATrackSim.Hough.secondStage=False \
        Trigger.FPGATrackSim.writeToAOD=True \
        Output.AODFileName="FPGATrackSimCITestAOD.root"
ls -l
echo "... analysis on RDO, this part is done ..."
 
root -b -q monitoring.root checkHist.C
echo "... rdo analysis output verification, this part is done ..."

cat << EOF > checkConvertedClusters.py
import ROOT
import numpy as np 

rootFile = ROOT.TFile.Open("FPGATrackSimCITestAOD.root")
clustersToCheck = ['xAODPixelClusters_1stFromFPGACluster',
                   'xAODStripClusters_1stFromFPGACluster']
tree = rootFile.Get("CollectionTree")

# Work around for ATEAM-1000
import cppyy.ll
cppyy.ll.cast["xAOD::PixelClusterContainer_v1"](0)

for branch in clustersToCheck:
    averageClustersPerEvent = np.mean([getattr(evt,branch).size() for evt in tree])
    if np.isclose(averageClustersPerEvent,0):
        raise ValueError(f"no recorded clusters in {branch}")
    else:
        print(f"there are on average {averageClustersPerEvent} clusters per event in {branch}")
EOF

echo "... verification of FPGATrackSim --> xAOD conversion"
python3 checkConvertedClusters.py

echo "...verification of FPGATrackSim --> xAOD conversion, this part is done ..."

echo "Running F-100 for a few ttbar events"
FPGATrackSim_F100.sh -t -n 2
ls -ltr
echo "done running F-100"
echo "... all done ..."
