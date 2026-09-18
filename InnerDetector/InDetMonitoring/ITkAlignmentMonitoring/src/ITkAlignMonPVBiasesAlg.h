/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

// **********************************************************************
// IDAlignMonPVBIases.cxx
// AUTHORS: Ambrosius  Vermeulen, Pierfrancesco Butti
// Adapted to new AthenaMT Monitoring 2022 by Per Johansson
// **********************************************************************

#ifndef ITkAlignMonPVBiasesAlg_H
#define ITkAlignMonPVBiasesAlg_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"
#include "TrkVertexFitterInterfaces/ITrackToVertexIPEstimator.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "GaudiKernel/ToolHandle.h"

#include <string.h>

class ISvcLocator;
class EventContext;
class StatusCode;

class ITkAlignMonPVBiasesAlg: public  AthMonitorAlgorithm {

public:
  ITkAlignMonPVBiasesAlg( const std::string & name, ISvcLocator* pSvcLocator );

  virtual ~ITkAlignMonPVBiasesAlg();
  virtual StatusCode initialize()override;
  virtual StatusCode fillHistograms( const EventContext& ctx ) const override;

private:
  ToolHandle<Trk::ITrackToVertexIPEstimator>  m_trackToVertexIPEstimator {this, "TrackToVertexIPEstimator", "Trk::TrackToVertexIPEstimator", ""};

  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackParticleName{this, "TrackParticleContainerName", "InDetTrackParticles","TrackPArticle Collection for PVBiases Monitoring"};
  SG::ReadHandleKey<xAOD::VertexContainer> m_vxContainerName{this,"vxContainerName","PrimaryVertices","Primary Vertices for PVBiases Monitoring"};
  
};

#endif
