/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigTrackPreSelHypoTool_H
#define TrigTrackPreSelHypoTool_H

#include "Gaudi/Property.h"
#include "xAODTau/TauJet.h"
#include "xAODTau/TauJetContainer.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "TrigCompositeUtils/HLTIdentifier.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"

#include "ITrigTrackPreSelHypoTool.h"

class StoreGateSvc;

namespace HLT {
  class TriggerElement;
}

class TrigTrackPreSelHypoTool : public extends<AthAlgTool, ITrigTrackPreSelHypoTool> {
 public:
  TrigTrackPreSelHypoTool( const std::string& type, 
           const std::string& name, 
           const IInterface* parent );

  virtual ~TrigTrackPreSelHypoTool();

  virtual StatusCode initialize() override;

  virtual StatusCode decide( std::vector<ITrigTrackPreSelHypoTool::TrackingInfo>& input )  const override;

  virtual bool decide( const ITrigTrackPreSelHypoTool::TrackingInfo& i ) const override;

 private:

  HLT::Identifier m_decisionId;

  ToolHandle<GenericMonitoringTool> m_monTool{ this, "MonTool", "", "Monitoring tool" };

};
#endif

