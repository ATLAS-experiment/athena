/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// HIGlobalAugmentationTool.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "HIGlobalAugmentationTool.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODHIEvent/HIEventShapeContainer.h"
#include <vector>
#include <format>

namespace DerivationFramework
{
  HIGlobalAugmentationTool::HIGlobalAugmentationTool( 	const std::string& t,
								const std::string& n,
								const IInterface* p ) :   base_class(t,n,p)
  {
    declareProperty("InDetTrackParticlesKey", m_TP_key="InDetTrackParticles");
    declareProperty("TrackSelectionTools", m_trkSelTools, "Track selection tools" );
    declareProperty("cutLevels", m_cutLevels, "Cut levels");
    declareProperty("nHarmonic", m_nHarmonic = 1, "Flow harmonic starting from v2" );
    declareProperty("doTopoClusDec", m_doTopoClusDec = false, "Decorate with CaloTopoCluster FCal cut, non-HI mode only" );
  }
  
  // Destructor
  HIGlobalAugmentationTool::~HIGlobalAugmentationTool() 
  {
  }  

  // Athena initialize and finalize
  StatusCode HIGlobalAugmentationTool::initialize()
  {
    unsigned int nTrackSels  = m_trkSelTools.size();
    for (unsigned int its = 0; its < nTrackSels; ++its) {      
      ATH_CHECK(m_trkSelTools[its].retrieve());
      m_decTrack_count.emplace_back(SG::AuxElement::Decorator<int>("TrackParticleMultiplicity_" + m_cutLevels[its] ));
    }
    //initilize decorators for flow vectors
    for (int vn = 1; vn <= m_nHarmonic; ++vn) {
      m_decFCalEtA_Qnx.emplace_back(SG::AuxElement::Decorator<float>(std::format("FCalEtA_Q{}x", vn + 1)));
      m_decFCalEtA_Qny.emplace_back(SG::AuxElement::Decorator<float>(std::format("FCalEtA_Q{}y", vn + 1)));
      m_decFCalEtC_Qnx.emplace_back(SG::AuxElement::Decorator<float>(std::format("FCalEtC_Q{}x", vn + 1)));
      m_decFCalEtC_Qny.emplace_back(SG::AuxElement::Decorator<float>(std::format("FCalEtC_Q{}y", vn + 1)));

      // half FCal is for FCal with eta > 4.0 only
      m_decHalfFCalEtA_Qnx.emplace_back(SG::AuxElement::Decorator<float>(std::format("HalfFCalEtA_Q{}x", vn + 1)));
      m_decHalfFCalEtA_Qny.emplace_back(SG::AuxElement::Decorator<float>(std::format("HalfFCalEtA_Q{}y", vn + 1)));
      m_decHalfFCalEtC_Qnx.emplace_back(SG::AuxElement::Decorator<float>(std::format("HalfFCalEtC_Q{}x", vn + 1)));
      m_decHalfFCalEtC_Qny.emplace_back(SG::AuxElement::Decorator<float>(std::format("HalfFCalEtC_Q{}y", vn + 1)));
    }
     
    
    return StatusCode::SUCCESS;
  }
  StatusCode HIGlobalAugmentationTool::finalize()
  {
    for (auto trkSelTool : m_trkSelTools ) { 
      ATH_CHECK(trkSelTool->finalize());
    }

    return StatusCode::SUCCESS;
  }

  StatusCode HIGlobalAugmentationTool::addBranches() const
  {

    //Load event EventInfo
    const xAOD::EventInfo* eventInfo = nullptr;     
    if(evtStore()->retrieve( eventInfo ).isFailure()){
      ATH_MSG_ERROR ("Cannot retrieve event info");
      return StatusCode::FAILURE;
    }
    
    //Load track particle container
    const xAOD::TrackParticleContainer* tracks = evtStore()->retrieve< const xAOD::TrackParticleContainer >(m_TP_key);
    if(!tracks) 
    {
      ATH_MSG_ERROR ("Couldn't retrieve TrackParticleContainer with key " << m_TP_key);
      return StatusCode::FAILURE;
    }

    //Count tracks
    unsigned int nTrackSels  = m_trkSelTools.size();
    std::vector <int> TrackParticleMultiplicity(nTrackSels,0);
    for(auto tp : *tracks)
    {
      if(tp)
      {
	      for (unsigned int its = 0; its < nTrackSels; ++its) {  
	        if(m_trkSelTools[its]->accept(*tp)) ++TrackParticleMultiplicity[its];
	      }  
      }
    }
    //decorate eventInfo with track multiplicity
    for (unsigned int its = 0; its < nTrackSels; ++its) {  
      (m_decTrack_count[its])(*eventInfo) = TrackParticleMultiplicity[its];    
    }
    
    float FCalEtA = 0;
    float FCalEtC = 0;
    
    std::vector<float> FCalEtA_Qnx(m_nHarmonic,0);
    std::vector<float> FCalEtA_Qny(m_nHarmonic,0);
    std::vector<float> FCalEtC_Qnx(m_nHarmonic,0);
    std::vector<float> FCalEtC_Qny(m_nHarmonic,0);

    float HalfFCalEtA = 0;
    float HalfFCalEtC = 0;

    std::vector<float> HalfFCalEtA_Qnx(m_nHarmonic,0);
    std::vector<float> HalfFCalEtA_Qny(m_nHarmonic,0);
    std::vector<float> HalfFCalEtC_Qnx(m_nHarmonic,0);
    std::vector<float> HalfFCalEtC_Qny(m_nHarmonic,0);
        
    // Set up the decorators for FCal Et
    SG::AuxElement::Decorator< float > decFCalEtA("FCalEtA");
    SG::AuxElement::Decorator< float > decFCalEtC("FCalEtC");    
    
    SG::AuxElement::Decorator< float > decHalfFCalEtA("HalfFCalEtA");
    SG::AuxElement::Decorator< float > decHalfFCalEtC("HalfFCalEtC");

    //Retrieve HIEventShape
    const xAOD::HIEventShapeContainer *eventShape = nullptr;      
    if(evtStore()->retrieve( eventShape, "HIEventShape" ).isFailure()){
      ATH_MSG_ERROR ("Cannot retrieve HIEventShape");
      return StatusCode::FAILURE;
    }
         
    //Calculate FCal A and FCal C sums 
    for(const auto* ptrEvtShp : *eventShape){
      if(ptrEvtShp->layer()!=21 && ptrEvtShp->layer()!=22 &&
        ptrEvtShp->layer()!=23) continue;

      float eta = ptrEvtShp->etaMin();
      const std::vector<float>&c1 = ptrEvtShp->etCos();
      const std::vector<float>&s1 = ptrEvtShp->etSin();

      if (eta > 0) {
        FCalEtA += ptrEvtShp->et();
        for (int vn = 1; vn <= m_nHarmonic; ++vn){    
            FCalEtA_Qnx.at(vn-1) += c1.at(vn);
            FCalEtA_Qny.at(vn-1) += s1.at(vn);
        } 
        if(eta > 4.0) {
          HalfFCalEtA += ptrEvtShp->et();
          for (int vn = 1; vn <= m_nHarmonic; ++vn){
            HalfFCalEtA_Qnx.at(vn-1) += c1.at(vn);
            HalfFCalEtA_Qny.at(vn-1) += s1.at(vn);
          }
        }
      } else if (eta < 0) {
          FCalEtC += ptrEvtShp->et();
          for (int vn = 1; vn <= m_nHarmonic; ++vn){    
              FCalEtC_Qnx.at(vn-1) += c1.at(vn);
              FCalEtC_Qny.at(vn-1) += s1.at(vn);
          }
          if(eta < -4.0){
            HalfFCalEtC += ptrEvtShp->et();
            for (int vn = 1; vn <= m_nHarmonic; ++vn){
              HalfFCalEtC_Qnx.at(vn-1) += c1.at(vn);
              HalfFCalEtC_Qny.at(vn-1) += s1.at(vn);
            }
	        }
        }
    } 

    if (m_doTopoClusDec) {
        // Setup the decorator for TopoCaloCluster cut
        // If this is true, the event is not compatible with UPC topologies in the FCal
        SG::AuxElement::Decorator< bool > decTopoClusterFCalCut("passUPCTopoCaloCut");
        //Default decoration set to false
        decTopoClusterFCalCut(*eventInfo) = false;

        //access topoClusters
        const xAOD::CaloClusterContainer *topos = 0;
        ATH_CHECK(evtStore()->retrieve(topos, "CaloCalTopoClusters"));
        bool hasTowerA{false};
        bool hasTowerC{false};
        for (const auto topo : *topos) {
          float topo_eta = topo->eta();
          if (abs(topo_eta) > 3.2 && abs(topo_eta) < 4.9) {
            float topo_pt = topo->pt() * 1e-3;
            if (topo_pt > 0.4) {
              if (topo_eta > 0) {
                hasTowerA = true;
              } else {
                hasTowerC = true;
              }
            }
          }
        }
	//decorate
	decTopoClusterFCalCut(*eventInfo) = (hasTowerA && hasTowerC);
    }

 
    //decorate
    decFCalEtA(*eventInfo) = FCalEtA;
    decFCalEtC(*eventInfo) = FCalEtC;
    decHalfFCalEtA(*eventInfo) = HalfFCalEtA;
    decHalfFCalEtC(*eventInfo) = HalfFCalEtC;

    for (int vn = 0; vn < m_nHarmonic; ++vn){ 
      (m_decFCalEtA_Qnx[vn])(*eventInfo) = FCalEtA_Qnx.at(vn);
      (m_decFCalEtA_Qny[vn])(*eventInfo) = FCalEtA_Qny.at(vn);
      (m_decFCalEtC_Qnx[vn])(*eventInfo) = FCalEtC_Qnx.at(vn);
      (m_decFCalEtC_Qny[vn])(*eventInfo) = FCalEtC_Qny.at(vn);
      (m_decHalfFCalEtA_Qnx[vn])(*eventInfo) = HalfFCalEtA_Qnx.at(vn);
      (m_decHalfFCalEtA_Qny[vn])(*eventInfo) = HalfFCalEtA_Qny.at(vn);
      (m_decHalfFCalEtC_Qnx[vn])(*eventInfo) = HalfFCalEtC_Qnx.at(vn);
      (m_decHalfFCalEtC_Qny[vn])(*eventInfo) = HalfFCalEtC_Qny.at(vn);
    }
        
    return StatusCode::SUCCESS;
  }  
  
}

