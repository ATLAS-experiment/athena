
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGBJETHYPO_TRIGBJETBTAGHYPOALG_H
#define TRIGBJETHYPO_TRIGBJETBTAGHYPOALG_H 1

#include "TrigBjetHypoAlgBase.h"
#include "TrigBjetBtagHypoTool.h"

#include <string>

#include "TrigCompositeUtils/TrigCompositeUtils.h"

#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"

#include "xAODBTagging/BTaggingAuxContainer.h"
#include "xAODBTagging/BTaggingContainer.h"

#include "AthenaMonitoringKernel/Monitored.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "AthContainers/ConstAccessor.h"


class TrigBjetBtagHypoAlg : public TrigBjetHypoAlgBase {
 public:
  TrigBjetBtagHypoAlg( const std::string& name, ISvcLocator* pSvcLocator );

  virtual StatusCode  initialize();
  virtual StatusCode  execute( const EventContext& context ) const;

 private:
  TrigBjetBtagHypoAlg();

  // online monitoring 
  virtual StatusCode monitor_jets( const ElementLinkVector<xAOD::JetContainer >& jetELs, const ElementLinkVector<xAOD::JetContainer >& all_bTaggedJetELs ) const ;
  virtual StatusCode monitor_tracks( const EventContext& context, const TrigCompositeUtils::DecisionContainer* prevDecisionContainer ) const;
  virtual StatusCode monitor_primary_vertex( const ElementLink< xAOD::VertexContainer >& primVertexEL ) const;
  virtual StatusCode monitor_flavor_probabilities( const ElementLinkVector< xAOD::JetContainer >& jetEL, const std::string& var_name) const;
  virtual StatusCode monitor_flavor_bb_probabilities( const ElementLinkVector< xAOD::JetContainer >& jetEL, const std::string& var_name) const;
  virtual ElementLinkVector<xAOD::JetContainer> collect_valid_links(const ElementLinkVector< xAOD::JetContainer >& jetEL, std::string tagger ) const;
  virtual StatusCode monitor_btagging( const ElementLinkVector< xAOD::JetContainer >& jetEL ) const;
  
 private:
  ToolHandleArray< TrigBjetBtagHypoTool > m_hypoTools {this,"HypoTools",{},"Hypo Tools"};
  ToolHandle<GenericMonitoringTool> m_monTool{this,"MonTool","","Monitoring tool"};
  
  SG::ReadHandleKey< xAOD::JetContainer > m_bTaggedJetKey {this,"BTaggedJetKey","","Key for b-tagged jets"};
  SG::ReadHandleKey< xAOD::TrackParticleContainer > m_trackKey {this,"TracksKey","","Key for precision tracks"};
  SG::ReadHandleKey< xAOD::VertexContainer > m_inputPrmVtx {this,"PrmVtxKey","","Key for Primary vertex collection for monitoring"};

  Gaudi::Property< std::string > m_prmVtxLink {this,"PrmVtxLink","Unspecified","Vertex Link name in navigation (input)"};

  SG::ReadCondHandleKey< InDet::BeamSpotData > m_beamSpotKey{ this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot" };

};

#endif

