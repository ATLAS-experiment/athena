/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigTLAMonitorAlgorithm.h"


TrigTLAMonitorAlgorithm::TrigTLAMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name,pSvcLocator)
{}

TrigTLAMonitorAlgorithm::~TrigTLAMonitorAlgorithm() {}


StatusCode TrigTLAMonitorAlgorithm::initialize() {
  ATH_CHECK( m_jetContainerKey          .initialize() );
  ATH_CHECK( m_pfjetContainerKey        .initialize() );
  ATH_CHECK( m_photonContainerKey       .initialize() );
  ATH_CHECK( m_muonContainerKey         .initialize() );
  ATH_CHECK( m_trackParticleContainerKey.initialize() );
  ATH_CHECK( m_tcEventInfoContainerKey  .initialize() );
  
  ATH_CHECK( m_trigDecisionTool.retrieve() );

  return AthMonitorAlgorithm::initialize();
}

/*************************************************
* Main filling function                          *
**************************************************/

StatusCode TrigTLAMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {
  // This is the method exectued on every event (inherited from AthMonitorAlgorithm)
  using namespace Monitored;

  //
  // print the trigger chain names 
  std::string chainName;
  
  int size_AllChains = m_allChains.size();
  ATH_MSG_DEBUG(" Size of the AllChains trigger container: " << size_AllChains );
  for (int i =0; i<size_AllChains; i++){
    chainName = m_allChains[i];
    ATH_MSG_DEBUG("  Chain number: " << i << " AllChains Chain Name: " << chainName );
  }

  //
  // Retrieve all the containers of monitored objects
  SG::ReadHandle<xAOD::TrigCompositeContainer> tcEventInfo;
  SG::ReadHandle<xAOD::JetContainer>           jets, pfjets;
  SG::ReadHandle<xAOD::PhotonContainer>        phs;
  SG::ReadHandle<xAOD::MuonContainer>          muons;
  SG::ReadHandle<xAOD::TrackParticleContainer> tracks;

  ANA_CHECK( readContainer<xAOD::TrigCompositeContainer>(tcEventInfo, m_tcEventInfoContainerKey,   ctx) );
  ANA_CHECK( readContainer<xAOD::JetContainer>          (jets,        m_jetContainerKey,           ctx) );
  ANA_CHECK( readContainer<xAOD::JetContainer>          (pfjets,      m_pfjetContainerKey,         ctx) );
  ANA_CHECK( readContainer<xAOD::PhotonContainer>       (phs,         m_photonContainerKey,        ctx) );
  ANA_CHECK( readContainer<xAOD::MuonContainer>         (muons,       m_muonContainerKey,          ctx) );
  ANA_CHECK( readContainer<xAOD::TrackParticleContainer>(tracks,      m_trackParticleContainerKey, ctx) );

  //
  // event-wide variables
  ANA_CHECK(fillEventInfoHistogram<float>(tcEventInfo, "AvgMu", "eventInfo" ));
  if (pfjets->size()>0){
    ATH_MSG_DEBUG("nEMPFlowJets = "<<pfjets->size());
    ANA_CHECK(fillEventInfoHistogram<int>(tcEventInfo, "NumPV", "eventInfo" ));
    ANA_CHECK(fillEventInfoHistogram<double>(tcEventInfo, "JetDensityEMPFlow", "eventInfo" ));
  }
  if (jets->size()>0){
    ATH_MSG_DEBUG("nEMTopoJets = "<<jets->size());
    ANA_CHECK(fillEventInfoHistogram<double>(tcEventInfo, "JetDensityEMTopo", "eventInfo" ));
  }

  const std::vector<std::string> jetCalibStates = {"JetConstitScaleMomentum_pt", "JetPileupScaleMomentum_pt", "JetEtaJESScaleMomentum_pt"};
  const std::vector<std::string> pfjetCalibStates = {"JetConstitScaleMomentum_pt", "JetPileupScaleMomentum_pt", "JetEtaJESScaleMomentum_pt", "JetGSCScaleMomentum_pt"};

  for ( const std::string& trigName : m_allChains ) {
    //Test if trigName has fired
    if(m_trigDecisionTool->isPassed(trigName, TrigDefs::requireDecision)){
      ATH_MSG_DEBUG("--->"<<trigName<<" fired!");
      // proceed filling the histogram
    
      //
      // jets
      using J = xAOD::Jet;
      ANA_CHECK(fillParticleHistograms<J>(jets, "jet", trigName));
      if (jets->size()>0){
        for (const auto& calibState: jetCalibStates){
          ANA_CHECK(fillJetPtCalibStatesHistograms(jets,calibState, "jet", trigName));
        }
        ANA_CHECK( (fillObjectVariableHistogram<J,float>(jets,"N90Constituents", "jet", trigName     )) );
        ANA_CHECK( (fillObjectVariableHistogram<J,float>(jets,"Timing",          "jet", trigName, -99)) );
        ANA_CHECK( (fillObjectVariableHistogram<J,float>(jets,"EMFrac",          "jet", trigName, -99)) );
        ANA_CHECK( (fillObjectVariableHistogram<J,float>(jets,"HECFrac",         "jet", trigName, -99)) );
      }

      //
      // particle flow jets
      ANA_CHECK(fillParticleHistograms<J>(pfjets, "pfjet", trigName));
      if (pfjets->size()>0){
        for (const auto& calibState: pfjetCalibStates){
          ANA_CHECK(fillJetPtCalibStatesHistograms(pfjets, calibState, "pfjet", trigName));
        }
        ANA_CHECK( fillJetTrackVariableHistogram<float>(pfjets,"TrackWidthPt1000",     "pfjet", trigName) );
        ANA_CHECK( fillJetTrackVariableHistogram<int>  (pfjets,"NumTrkPt1000",         "pfjet", trigName) );
        ANA_CHECK( fillJetTrackVariableHistogram<float>(pfjets,"SumPtTrkPt500",        "pfjet", trigName) );
        ANA_CHECK( fillJetTrackVariableHistogram<float>(pfjets,"SumPtChargedPFOPt500", "pfjet", trigName) );
        ANA_CHECK( (fillObjectVariableHistogram<J,float>(pfjets,"ActiveArea",          "pfjet", trigName)) );
        ANA_CHECK( (fillObjectVariableHistogram<J,float>(pfjets,"Jvt",                 "pfjet", trigName, -99)) );
        ANA_CHECK( (fillObjectVariableHistogram<J,float>(pfjets,"JvtRpt",              "pfjet", trigName, -99)) );
        ANA_CHECK( (fillObjectVariableHistogram<J,float>(pfjets,"fastDIPS20211215_pu", "pfjet", trigName)) );
        ANA_CHECK( (fillObjectVariableHistogram<J,float>(pfjets,"fastDIPS20211215_pb", "pfjet", trigName)) );
        ANA_CHECK( (fillObjectVariableHistogram<J,float>(pfjets,"fastDIPS20211215_pc", "pfjet", trigName)) );
        ANA_CHECK( (fillObjectVariableHistogram<J,float>(pfjets,"GN120230331_pu",      "pfjet", trigName)) );
        ANA_CHECK( (fillObjectVariableHistogram<J,float>(pfjets,"GN120230331_pb",      "pfjet", trigName)) );
        ANA_CHECK( (fillObjectVariableHistogram<J,float>(pfjets,"GN120230331_pc",      "pfjet", trigName)) );
      }

      //
      // photons
      ATH_CHECK(fillParticleHistograms<xAOD::Photon>(phs, "ph", trigName));

      //
      // muons
      ANA_CHECK(fillParticleHistograms<xAOD::Muon>(muons, "muon", trigName));

      //
      // Tracks
      using TP = xAOD::TrackParticle;
      if (tracks->size()>0 && trigName.find("pf_ftf") != std::string::npos) {
        ANA_CHECK( fillParticleHistograms<TP>(tracks, "trk", trigName) );
        ANA_CHECK( (fillObjectVariableHistogram<TP,float>(tracks, "qOverP",                       "trk", trigName)) );
        ANA_CHECK( (fillObjectVariableHistogram<TP,float>(tracks, "chiSquared",                   "trk", trigName)) );
        ANA_CHECK( (fillObjectVariableHistogram<TP,float>(tracks, "numberDoF",                    "trk", trigName)) );
        ANA_CHECK( (fillObjectVariableHistogram<TP,float>(tracks, "btagIp_d0",                    "trk", trigName)) );
        ANA_CHECK( (fillObjectVariableHistogram<TP,float>(tracks, "btagIp_d0Uncertainty",         "trk", trigName)) );
        ANA_CHECK( (fillObjectVariableHistogram<TP,float>(tracks, "btagIp_z0SinTheta",            "trk", trigName)) );
        ANA_CHECK( (fillObjectVariableHistogram<TP,float>(tracks, "btagIp_z0SinThetaUncertainty", "trk", trigName)) );
      }

      //
      // Delta Angles
      if(jets->size()>=2) {
        ANA_CHECK(fillDeltaRHistograms(jets  ->at(0), jets  ->at(1), "jet0jet1"    , trigName));
      }

      if(pfjets->size()>=2) {
        ANA_CHECK(fillDeltaRHistograms(pfjets->at(0), pfjets->at(1), "pfjet0pfjet1", trigName));
      }

      if(jets->size()>=1 && phs->size()>=1) {
        ANA_CHECK(fillDeltaRHistograms(jets  ->at(0), phs   ->at(0), "jet0ph0"     , trigName));
      }

      if(pfjets->size()>=1 && phs->size()>=1) {
        ANA_CHECK(fillDeltaRHistograms(pfjets->at(0), phs   ->at(0), "pfjet0ph0"   , trigName));
      }
    }
  } // for AllChains

  return StatusCode::SUCCESS;
}


/*************************************************
* Implementation of specialized fillers          *
**************************************************/

StatusCode TrigTLAMonitorAlgorithm::fillDeltaRHistograms(const xAOD::IParticle* p0, const xAOD::IParticle* p1, const std::string& prefix, const std::string& trigName) const
{
  // histograms
  Monitored::Scalar<double> dr(prefix+"dr_"  +trigName,0.0);

  // fill
  dr = p0->p4().DeltaR(p1->p4());
  fill("TrigTLAMonitor", dr);

  return StatusCode::SUCCESS;
}


StatusCode TrigTLAMonitorAlgorithm::fillJetPtCalibStatesHistograms(SG::ReadHandle<xAOD::JetContainer> jets,  const std::string& calibState,  const std::string& prefix,  const std::string& trigName) const {

  Monitored::Scalar<float> ptCalibScale  (prefix+calibState+"_"+trigName,-1.0);

  unsigned cnt(0);
  for(auto jet : *jets) {
    auto status = jet->getAttribute<float>(calibState, ptCalibScale);
    if (!status){
      ATH_MSG_WARNING("Failed retrieving "<<calibState<<" for "<<prefix);
    }
    ptCalibScale = ptCalibScale*1e-3;
    fill("TrigTLAMonitor",ptCalibScale);
    if (cnt < 3) ATH_MSG_DEBUG(prefix<<" "<<calibState<<" = "<<ptCalibScale<<" GeV");
    cnt++;
  }

  return StatusCode::SUCCESS;
}
