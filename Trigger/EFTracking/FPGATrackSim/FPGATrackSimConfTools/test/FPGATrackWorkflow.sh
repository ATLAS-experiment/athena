#!/bin/bash

# get the file from CERNBOX
# this is temporary measure, the test will switch to file on CVMFS once there is one available
if [ ! -f rdo_small.root ]
then
    curl https://cernbox.cern.ch/remote.php/dav/public-files/12CCGeTUn0I3MLv/eos/user/t/tbold/EFTracking/rdo_small.root --output rdo_small.root
    if [ ! -f rdo_small.root ] 
    then
        echo "Could not fetch the input rdo file, exiting, ..."
        return 1 2> /dev/null || exit 1    
    fi
    echo "A small input RDO has been downloaded, ..."
else
    echo "A small input RDO has been downloaded previously, ..."
fi

GEO_TAG="ATLAS-P2-RUN4-03-00-00"

#make wrapper file
Reco_tf.py --CA \
    --steering doRAWtoALL \
    --preExec "flags.Trigger.FPGATrackSim.wrapperFileName='wrapper.root'" \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateTracksFlags" \
    --postInclude "FPGATrackSimSGInput.FPGATrackSimSGInputConfig.FPGATrackSimSGInputCfg" \
    --inputRDOFile rdo_small.root \
    --outputAODFile AOD.pool.root \
    --maxEvents -1 &&
ls -l &&

# generate maps
python -m FPGATrackSimConfTools.FPGATrackSimMapMakerConfig \
--filesInput=wrapper.root \
OutFileName="MyMaps_" \
region=0 \
GeoModel.AtlasVersion="ATLAS-P2-RUN4-03-00-00" &&
ls -l &&

mkdir -p maps &&
mv MyMaps_region0.rmap maps/eta0103phi0305.rmap &&
mv *.rmap maps/eta0103phi0305.subrmap &&
mv MyMaps_region0.pmap maps/pmap &&
touch maps/moduleidmap &&

python -m FPGATrackSimBankGen.FPGATrackSimBankGenConfig \
--filesInput=rdo_small.root \
Trigger.FPGATrackSim.mapsDir=maps  &&
ls -l &&

### The above bank is great but is too small to make fit constants
# get the file from CERNBOX
# this is temporary measure, the test will switch to file on CVMFS once there is one available
if [ ! -f combined_input_matrix.root ]
then
    curl https://cernbox.cern.ch/remote.php/dav/public-files/WRZ2LVU6K4P8iQV/combined_matrix.root --output combined_input_matrix.root
    if [ ! -f combined_input_matrix.root ] 
    then
        echo "Could not fetch the input matrix file, exitting, ..."
        return 1 2> /dev/null || exit 1    
    fi
    echo "A matrix file has been downloaded, ..."
else
    echo "A matrix file has been downloaded previously, ..."
fi

### Run const generation on combined matrix file
python -m FPGATrackSimBankGen.FPGATrackSimBankConstGenConfig \
    Trigger.FPGATrackSim.FPGATrackSimMatrixFileRegEx='./combined_input_matrix.root' \
    Trigger.FPGATrackSim.mapsDir=./maps/ \
    --evtMax=1 && \
ls -l &&

mkdir -p banks && 
mv sectors* slices* corr* const.root combined_matrix.root banks &&

# # run analysis
python -m FPGATrackSimConfTools.FPGATrackSimAnalysisConfig \
Trigger.FPGATrackSim.wrapperFileName="wrapper.root" \
Trigger.FPGATrackSim.mapsDir=./maps \
Trigger.FPGATrackSim.tracking=True \
Trigger.FPGATrackSim.bankDir=./banks/ &&
ls -l &&

python -m FPGATrackSimConfTools.FPGATrackSimAnalysisConfig \
--filesInput=rdo_small.root \
Trigger.FPGATrackSim.mapsDir=./maps \
Trigger.FPGATrackSim.tracking=True \
Trigger.FPGATrackSim.sampleType='skipTruth' \
Trigger.FPGATrackSim.bankDir=./banks/ &&
ls -l &&
 
cat << EOF > checkHist.C
{
    _file0->cd("FPGATrackSimLogicalHitsProcessAlg");
    TH1* h = (TH1*)gDirectory->Get("nroads_1st"); 
    h->Print(); 
    if ( h->GetEntries() == 0 ) {
        throw std::runtime_error("oh dear, after all of this there are no roads");
    }
}
EOF

root -b -q monitoring.root checkHist.C
