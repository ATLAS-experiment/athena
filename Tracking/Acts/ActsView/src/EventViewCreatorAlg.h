/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EVENT_VIEW_CREATOR_ALG_H
#define EVENT_VIEW_CREATOR_ALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "ActsView/IRoICreatorTool.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "StoreGate/WriteHandleKey.h"
#include "AthContainers/ConstDataVector.h"
#include "AthLinks/ElementLink.h"
#include "AthViews/View.h"

class EventViewCreatorAlg : public AthReentrantAlgorithm {
 public:
  EventViewCreatorAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~EventViewCreatorAlg() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
  StatusCode placeRoIInView( const ElementLink<TrigRoiDescriptorCollection>& roiEL, 
			     SG::View* view, 
			     const EventContext& ctx ) const;

 private:
  ToolHandle< IRoICreatorTool > m_roiTool {this, "RoICreatorTool", "", "Tool for creating RoIs"};

  SG::WriteHandleKey< ViewContainer > m_viewsKey {this, "Views", "",
      "The key of views collection produced" };
  SG::WriteHandleKey< ConstDataVector< TrigRoiDescriptorCollection > > m_inViewRoIs {this, "InViewRoIs", "",
      "RoIs in the View"};
};

#endif
