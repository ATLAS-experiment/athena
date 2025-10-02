///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// GSCCalibStep.cxx 
// Implementation file for class GSCCalibStep
/////////////////////////////////////////////////////////////////// 

#include "JetCalibTools/GSCCalibStep.h"
#include "PathResolver/PathResolver.h"
#include "AsgDataHandles/ReadDecorHandle.h"

GSCCalibStep::GSCCalibStep(const std::string& name)
  : asg::AsgTool( name ){ }



/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

StatusCode GSCCalibStep::initialize() {
  ATH_MSG_DEBUG ("Initializing " << name() );

  ATH_MSG_DEBUG("Reading from " << m_jetInScale << " and writing to " << m_jetOutScale);

  ATH_CHECK( m_histTool_EM3.retrieve());
  ATH_CHECK( m_histTool_ChargedFraction.retrieve());
  ATH_CHECK( m_histTool_Tile0.retrieve());
  ATH_CHECK( m_histTool_PunchThrough.retrieve());
  ATH_CHECK( m_histTool_nTrk.retrieve());
  ATH_CHECK( m_histTool_trackWIDTH.retrieve());
 
  return StatusCode::SUCCESS;
}


StatusCode GSCCalibStep::calibrate(xAOD::JetContainer& jets) const {

  ATH_MSG_DEBUG("calibrating jet collection.");

  for (xAOD::Jet* jet : jets){ 

    JetHelper::JetContext jc;

    xAOD::JetFourMom_t jetconstitP4 = jet->getAttribute<xAOD::JetFourMom_t>("JetConstitScaleMomentum");
    std::vector<float> samplingFrac = jet->getAttribute<std::vector<float> >("EnergyPerSampling");
    // get Primary Vertex index
    int PVindex = 0;
    ATH_MSG_DEBUG("PV index:" << PVindex);
    // get detector Eta
    float detectorEta = jet->getAttribute<float>("DetectorEta");
    // get trackWIDTHPVX
    float trackWIDTHPVX = 0;
    static const SG::ConstAccessor<std::vector<float> > TrackWidthPt1000Acc ("TrackWidthPt1000");
    if(TrackWidthPt1000Acc.isAvailable(*jet))
    {
        trackWIDTHPVX = TrackWidthPt1000Acc(*jet).at(PVindex);
        ATH_MSG_DEBUG("trackWIDTHPVX found set to: " << trackWIDTHPVX);
    }
    jc.setValue("trackWIDTH", trackWIDTHPVX);
    // get nTrkPVX
    int nTrkPVX = 0;
    static const SG::ConstAccessor<std::vector<int>> NumTrkPt1000Acc ("NumTrkPt1000");
    if(NumTrkPt1000Acc.isAvailable(*jet))
    {
        nTrkPVX = NumTrkPt1000Acc(*jet).at(PVindex);
        ATH_MSG_DEBUG("nTrkPVX found set to: " << nTrkPVX);
    }
    jc.setValue("nTrk", nTrkPVX);
    // get Charged Fraction
    float ChargedFraction = 0;
    static const SG::ConstAccessor<std::vector<float>> SumPtChargedPFOPt500Acc ("SumPtChargedPFOPt500");
    if(SumPtChargedPFOPt500Acc.isAvailable(*jet))
    {
        ChargedFraction = SumPtChargedPFOPt500Acc(*jet).at(PVindex)/jetconstitP4.Pt();
        ATH_MSG_DEBUG("ChargedFraction found set to: " << ChargedFraction);
    }
    jc.setValue("ChargedFraction", ChargedFraction);
    // get EM3
    float EM3 = (samplingFrac[3]+samplingFrac[7])/jetconstitP4.e();
    ATH_MSG_DEBUG("EM3 found set to: " << EM3);
    jc.setValue("EM3", EM3);
    // get Tile0
    float Tile0 = (samplingFrac[12]+samplingFrac[18])/jetconstitP4.e();
    ATH_MSG_DEBUG("Tile0 found set to: " << Tile0);
    jc.setValue("Tile0", Tile0);
    // get N90Constituents
    double N90Constituents = 0;
    static const SG::ConstAccessor<float> N90ConstituentsAcc ("N90Constituents");
    if(N90ConstituentsAcc.isAvailable(*jet))
    {
        N90Constituents = N90ConstituentsAcc(*jet);
        ATH_MSG_DEBUG("N90Constituents found set to: " << N90Constituents);
    }
    jc.setValue("N90Constituents", N90Constituents);
    // get caloWIDTH
    double caloWIDTH = 0;
    static const SG::ConstAccessor<float> WidthAcc ("Width");
    if(WidthAcc.isAvailable(*jet))
    {
        caloWIDTH = WidthAcc(*jet);
        ATH_MSG_DEBUG("caloWIDTH found set to: " << caloWIDTH);
    }
    jc.setValue("caloWIDTH", caloWIDTH);
    // get TG3
    float TG3 = (samplingFrac[17])/jetconstitP4.e();
    ATH_MSG_DEBUG("TG3 found set to: " << TG3);
    jc.setValue("TG3", TG3);
    // get Muon segments
    int Nsegments = 0;
    static const SG::ConstAccessor<int> GhostMuonSegmentCountAcc ("GhostMuonSegmentCount");
    if(GhostMuonSegmentCountAcc.isAvailable(*jet))
    {
        Nsegments = GhostMuonSegmentCountAcc(*jet);
        ATH_MSG_DEBUG("Nsegments found set to: " << Nsegments);
    }
    jc.setValue("Nsegments", Nsegments);

    float getGSCCorrection = 1.0;
    int etabin = fabs(detectorEta)/0.1;// m_binSize in old version

    const xAOD::JetFourMom_t startingP4 = jet->getAttribute<xAOD::JetFourMom_t>(m_jetInScale);
    jet->setJetP4(startingP4);

    ATH_MSG_DEBUG("Jet pt original ("<<m_jetInScale<<"): " << jet->pt()*1e-3);

    ATH_MSG_DEBUG("ChargedFraction Response: " << getChargedFractionResponse(*jet, jc, etabin));
    ATH_MSG_DEBUG("Tile0 Response: " <<getTile0Response(*jet, jc, etabin));
    ATH_MSG_DEBUG("EM3 Response: " << getEM3Response(*jet, jc, etabin));
    ATH_MSG_DEBUG("NTrk Response: " <<getNTrkResponse(*jet, jc, etabin));
    ATH_MSG_DEBUG("TrkWidth Response: " <<getTrackWIDTHResponse(*jet, jc, etabin));

    getGSCCorrection*=1./getChargedFractionResponse(*jet, jc, etabin);
    jet->setJetP4( startingP4*getGSCCorrection );
    getGSCCorrection*=1./getTile0Response(*jet, jc, etabin); 
    jet->setJetP4( startingP4*getGSCCorrection );
    getGSCCorrection*=1./getEM3Response(*jet, jc, etabin);
    jet->setJetP4( startingP4*getGSCCorrection );
    getGSCCorrection*=1./getNTrkResponse(*jet, jc, etabin);
    jet->setJetP4( startingP4*getGSCCorrection );
    getGSCCorrection*=1./getTrackWIDTHResponse(*jet, jc, etabin);

    ATH_MSG_DEBUG("GSC full correction: " << getGSCCorrection);

    jet->setAttribute<xAOD::JetFourMom_t>(m_jetOutScale,startingP4*getGSCCorrection);
    jet->setJetP4( startingP4*getGSCCorrection );

    ATH_MSG_DEBUG("Jet pt calibrated:" << jet->pt()*1e-3);


  }// loop jets

  return StatusCode::SUCCESS;
}

float GSCCalibStep::getChargedFractionResponse(const xAOD::Jet& jet, const JetHelper::JetContext& jc, uint etabin) const {
  if (jc.getValue<float>("ChargedFraction")<=0) return 1; //ChargedFraction < 0 is unphysical, ChargedFraction = 0 is a special case, so we return 1 for ChargedFraction <= 0
  if ( etabin >= m_histTool_ChargedFraction.size() ) return 1.;
  double ChargedFractionResponse = m_histTool_ChargedFraction[etabin]->getValue(jet, jc);
  return ChargedFractionResponse;
}

float GSCCalibStep::getTile0Response(const xAOD::Jet& jet, const JetHelper::JetContext& jc, uint etabin) const {
  if (jc.getValue<float>("Tile0")<0) return 1; //Tile0 < 0 is unphysical, so we return 1
  if ( etabin >= m_histTool_Tile0.size() ) return 1.;
  double Tile0Response = m_histTool_Tile0[etabin]->getValue(jet, jc);
  return Tile0Response;
}

float GSCCalibStep::getEM3Response(const xAOD::Jet& jet, const JetHelper::JetContext& jc, uint etabin) const {
  if (jc.getValue<float>("EM3")<=0) return 1; //EM3 < 0 is unphysical, EM3 = 0 is a special case, so we return 1 for EM3 <= 0
  if ( etabin >= m_histTool_EM3.size() ) return 1.;
  float EM3Response = m_histTool_EM3[etabin]->getValue(jet, jc);
  return EM3Response;
}

float GSCCalibStep::getPunchThroughResponse(const xAOD::Jet& jet, const JetHelper::JetContext& jc, double eta_det) const {
  int etabin=-99;
  std::vector<float> punchThroughEtaBins = {0.0, 1.3, 1.9};//variable in old version

  if (punchThroughEtaBins.empty() || m_histTool_PunchThrough.size() != punchThroughEtaBins.size()-1) 
    ATH_MSG_WARNING("Please check that the punch through eta binning is properly set in your config file");
  if ( eta_det >= punchThroughEtaBins.back() || jc.getValue<float>("Nsegments") < 20 ) return 1;
  for (uint i=0; i<punchThroughEtaBins.size()-1; ++i) {
    if(eta_det >= punchThroughEtaBins[i] && eta_det < punchThroughEtaBins[i+1]) etabin = i;
  }
  if(etabin<0) {
    ATH_MSG_WARNING("There was a problem determining the eta bin to use for the punch through correction.");
    //this could probably be improved, but to avoid a seg fault...
    return 1;
  }
  double PunchThroughResponse = m_histTool_PunchThrough[etabin]->getValue(jet, jc);
  if(PunchThroughResponse>1) return 1;
  return PunchThroughResponse;
}

float GSCCalibStep::getNTrkResponse(const xAOD::Jet& jet, const JetHelper::JetContext& jc, uint etabin) const {
  if (jc.getValue<int>("nTrk")<=0) return 1; //nTrk < 0 is unphysical, nTrk = 0 is a special case, so return 1 for nTrk <= 0
  if ( etabin >= m_histTool_nTrk.size() ) return 1.;
  double nTrkResponse;
  /*if(m_turnOffTrackCorrections){
    if(pT>=m_turnOffStartingpT && pT<=m_turnOffEndpT){
      double responseatStartingpT = m_histTool_nTrk[etabin]->getValue(jet, jc);
      nTrkResponse = (1-responseatStartingpT)/(m_turnOffEndpT-m_turnOffStartingpT);
      nTrkResponse *= pT;
      nTrkResponse += 1 - (m_turnOffEndpT*(1-responseatStartingpT)/(m_turnOffEndpT-m_turnOffStartingpT));
      return nTrkResponse;
    }
    else if(pT>m_turnOffEndpT) return 1;
  } TODO */
  nTrkResponse = m_histTool_nTrk[etabin]->getValue(jet, jc);
  return nTrkResponse;
}

float GSCCalibStep::getTrackWIDTHResponse(const xAOD::Jet& jet, const JetHelper::JetContext& jc, uint etabin) const {
  if (jc.getValue<float>("trackWIDTH")<=0) return 1;
  if ( etabin >= m_histTool_trackWIDTH.size() ) return 1.;
  //jets with no tracks are assigned a trackWIDTH of -1, we use the trackWIDTH=0 correction in those cases
  double trackWIDTHResponse;
  /*if(m_turnOffTrackCorrections){
    if(pT>=m_turnOffStartingpT && pT<=m_turnOffEndpT){
      double responseatStartingpT = readPtJetPropertyHisto(m_turnOffStartingpT, trackWIDTH, *m_respFactorstrackWIDTH[etabin]);
      trackWIDTHResponse = (1-responseatStartingpT)/(m_turnOffEndpT-m_turnOffStartingpT);
      trackWIDTHResponse *= pT;
      trackWIDTHResponse += 1 - (m_turnOffEndpT*(1-responseatStartingpT)/(m_turnOffEndpT-m_turnOffStartingpT));
      return trackWIDTHResponse;
    }
    else if(pT>m_turnOffEndpT) return 1;
  } TODO */
  trackWIDTHResponse = m_histTool_trackWIDTH[etabin]->getValue(jet, jc);
  return trackWIDTHResponse;
}

