#!/bin/bash

# get the file from CERNBOX
# this is temporary measure, the test will switch to file on CVMFS once there is one available
if [ ! -f rdo_small.root ]
then
    curl https://cernbox.cern.ch/remote.php/dav/public-files/12CCGeTUn0I3MLv/eos/user/t/tbold/EFTracking/rdo_small.root --output rdo_small.root
    if [ ! -f rdo_small.root ] 
    then
        echo "Could not fetch the input rdo file, exitting, ..."
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
mv MyMaps_region0_stripbarrel0__trim0p100_NSlices-10.rmap maps/eta0103phi0305.subrmap &&
mv MyMaps_region0.pmap maps/pmap &&
mv MyMaps_region0.rmap maps/eta0103phi0305.rmap &&
touch maps/moduleidmap &&

# banks generation
python -m FPGATrackSimBankGen.FPGATrackSimBankGenConfig \
--filesInput=rdo_small.root \
Trigger.FPGATrackSim.mapsDir=maps  &&
ls -l &&

# # run analysis
python -m FPGATrackSimConfTools.FPGATrackSimAnalysisConfig \
Trigger.FPGATrackSim.wrapperFileName="wrapper.root" \
Trigger.FPGATrackSim.mapsDir=./maps &&
ls -l &&

 
cat << EOF > checkHist.C
{
    _file0->cd("FPGATrackSimLogicalHitsProcessAlg");
    TH1* h = (TH1*)gDirectory->Get("nroads_1st"); 
    h->Print(); 
    if ( h->GetEntries() == 0 ) {
        throw std::runtime_error("oh deear, after all of this there are no roads");
    }
}
EOF

root -b -q monitoring.root checkHist.C