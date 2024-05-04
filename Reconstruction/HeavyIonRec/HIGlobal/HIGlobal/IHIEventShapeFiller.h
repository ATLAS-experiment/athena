/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef __INTERFACE_HIEVENTSHAPEMODIFIER_H__
#define __INTERFACE_HIEVENTSHAPEMODIFIER_H__

#include "AsgTools/IAsgTool.h"
#include "xAODHIEvent/HIEventShapeContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include <string>
#include <NavFourMom/INavigable4MomentumCollection.h>
#include <CaloEvent/CaloCellContainer.h>

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include <iostream>
#include <iomanip>
#include <memory>

class CaloCellContainer;

class IHIEventShapeFiller : virtual public asg::IAsgTool
{
  ASG_TOOL_INTERFACE(IHIEventShapeFiller)
public:
  virtual ~IHIEventShapeFiller() {};

  virtual StatusCode initializeIndex() = 0;
  virtual StatusCode initializeEventShapeContainer(std::unique_ptr<xAOD::HIEventShapeContainer>& evtShape) const = 0;
  
  virtual StatusCode fillCollectionFromTowers(std::unique_ptr<xAOD::HIEventShapeContainer>& evtShape, const SG::ReadHandleKey<xAOD::CaloClusterContainer>& m_tower_container_key, const SG::ReadHandleKey<INavigable4MomentumCollection>& m_navi_container_key, const EventContext& ctx) const = 0;

  virtual StatusCode fillCollectionFromCells(std::unique_ptr<xAOD::HIEventShapeContainer>& evtShape, const SG::ReadHandleKey<CaloCellContainer>& m_cell_container_key, const EventContext& ctx) const = 0;

  inline const std::string& getContainerName() const { return m_outputContainerName; };
  inline void setContainerName(const std::string& cname) { m_outputContainerName = cname; };

private:

  std::string m_outputContainerName;

};

#endif
