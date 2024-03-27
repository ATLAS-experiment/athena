#!/bin/bash
set -e

RDO=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.900498.PG_single_muonpm_Pt100_etaFlatnp0_43.recon.RDO.e8481_s4149_r14697/RDO.33675668._000016.pool.root.1
RDO_EVT=200
GEO_TAG="ATLAS-P2-RUN4-03-00-00"

#make wrapper file
echo "... RDO to AOD with sim"
Reco_tf.py --CA \
    --steering doRAWtoALL \
    --preExec "flags.Trigger.FPGATrackSim.wrapperFileName='wrapper.root'" \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateTracksFlags" \
    --postInclude "FPGATrackSimSGInput.FPGATrackSimSGInputConfig.FPGATrackSimSGInputCfg" \
    --inputRDOFile ${RDO} \
    --outputAODFile AOD.pool.root \
    --maxEvents ${RDO_EVT}
ls -l
echo "... RDO to AOD with sim, this part is done ..."

# generate maps
echo "... Maps Making"
python -m FPGATrackSimConfTools.FPGATrackSimMapMakerConfig \
--filesInput=wrapper.root \
OutFileName="MyMaps_" \
region=0 \
GeoModel.AtlasVersion="ATLAS-P2-RUN4-03-00-00"
ls -l
echo "... Maps Making, this part is done ..."

mkdir -p maps
mv MyMaps_region0.rmap maps/eta0103phi0305.rmap
mv *.rmap maps/eta0103phi0305.subrmap
mv MyMaps_region0.pmap maps/pmap
touch maps/moduleidmap

echo "... Banks generation"
python -m FPGATrackSimBankGen.FPGATrackSimBankGenConfig \
--filesInput=${RDO} \
--evtMax=${RDO_EVT} \
Trigger.FPGATrackSim.mapsDir=maps 
ls -l
echo "... Banks generation, this part is done ..."

echo "... const generation on combined matrix file"
python -m FPGATrackSimBankGen.FPGATrackSimBankConstGenConfig \
    Trigger.FPGATrackSim.FPGATrackSimMatrixFileRegEx='/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/ATLAS-P2-RUN4-03-00-00/combined_matrix.root' \
    Trigger.FPGATrackSim.mapsDir=./maps/ \
    --evtMax=1
ls -l
echo "... const generation on combined matrix file, this part is done ..."

mkdir -p banks 
mv sectors* slices* corr* const.root combined_matrix.root banks

echo "... analysis on wrapper"
python -m FPGATrackSimConfTools.FPGATrackSimAnalysisConfig \
Trigger.FPGATrackSim.wrapperFileName="wrapper.root" \
Trigger.FPGATrackSim.mapsDir=./maps \
Trigger.FPGATrackSim.tracking=True \
Trigger.FPGATrackSim.bankDir=./banks/
ls -l
echo "... analysis on wrapper, this part is done ..."

echo "... analysis on RDO"
python -m FPGATrackSimConfTools.FPGATrackSimAnalysisConfig \
--filesInput=${RDO} \
--evtMax=${RDO_EVT} \
Trigger.FPGATrackSim.mapsDir=./maps \
Trigger.FPGATrackSim.tracking=True \
Trigger.FPGATrackSim.sampleType='skipTruth' \
Trigger.FPGATrackSim.bankDir=./banks/
ls -l
echo "... analysis on RDO, this part is done ..."
 

echo "... analysis output verification"
cat << EOF > checkHist.C
{
    _file0->cd("FPGATrackSimLogicalHitsProcessAlg");
    TH1* h = (TH1*)gDirectory->Get("nroads_1st");
    if ( h == nullptr )
        throw std::runtime_error("oh dear, after all of this there are no roads histogram");
    h->Print(); 
    if ( h->GetEntries() == 0 ) {
        throw std::runtime_error("oh dear, after all of this there are zero roads");
    }
}
EOF

root -b -q monitoring.root checkHist.C
echo "... analysis output verification, this part is done ..."
echo "... all done ..."
