/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


/** @file ITkGlobalLRTMonAlg.h
 * Implementation of inner detector global track monitoring tool
 *
 *@author
 * Leonid Serkin <lserkin@cern.ch> @n
 * 
 * based on InDetGlobalLRTMonTool.h 
 * 
 ****************************************************************************/

#ifndef ITkGlobalLRTMonAlg_H
#define ITkGlobalLRTMonAlg_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"

#include "GaudiKernel/EventContext.h"


#include "StoreGate/ReadHandleKey.h"

//------------TrackMon------------
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"

#include "xAODTracking/TrackParticleContainer.h"

//Standard c++
#include <string>
#include <memory>



//------------------------------

namespace InDet {
  class IInDetTrackSelectionTool;
}


class ITkGlobalLRTMonAlg : public AthMonitorAlgorithm {
  
 public:
  
  ITkGlobalLRTMonAlg( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~ITkGlobalLRTMonAlg();
  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms( const EventContext& ctx ) const override;
  std::string findComponentString(int bec, int ld) const;
  
 private:
  
  ToolHandle <InDet::IInDetTrackSelectionTool> m_trackSelTool; // baseline

  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackParticleName{this, "TrackParticleContainerName", "InDetLargeD0TrackParticles","TrackParticle Collection for Global Monitoring"};


  
};
#endif
